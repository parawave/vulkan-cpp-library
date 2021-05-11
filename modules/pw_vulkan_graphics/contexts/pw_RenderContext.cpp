/*
  ==============================================================================

   This file is part of the Parawave Vulkan C++ library.

   The code included in this file is provided under the terms of the ISC license
   https://opensource.org/licenses/ISC.

   Copyright (c) 2021 - Parawave Audio (https://parawave-audio.com/vulkan-cpp-library)

   Permission to use, copy, modify, and/or distribute this software for any 
   purpose with or without fee is hereby granted, provided that the above 
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES 
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF 
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
   SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
   OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

namespace parawave
{

//==============================================================================
/** A RenderContext manages the state of one active VulkanContext and holds the swapchain frames
    and acquire one frame per render cycle. */
class RenderContext : public DeviceState
{
public:
    using FrameType = FrameState;
    using OverlayType = OverlayState;

private:
    static constexpr uint32_t maxFramesInFlight = 1;

public:
    enum class DrawStatus
    {
        hasFailed = 0,
        hasFinished,
        needsSwapChainRecreation
    };

public:
    RenderContext(VulkanDevice& device, const VulkanSwapchain& swapchain_) : 
        DeviceState(device, swapchain_.getImageFormat()), swapchain(swapchain_)
    {
        // In case the RenderContext is recreated, minimize the storage used by previous allocations!
        minimizeStorage();

        // Per Frame Resources
        for (auto i = 0U; i < maxFramesInFlight; ++i)
        {
            imageAcquiredSemaphores.add(new VulkanSemaphore(device));
            imageCompletedFences.add(new VulkanFence(device));
        }

        const auto frameBufferFormat = swapchain.getImageFormat();

        for (auto i = 0U; i < maxFramesInFlight; ++i)
            frames.add(new FrameType(*this, swapchain.getWidth(), swapchain.getHeight(), frameBufferFormat));

        for (auto i = 0U; i < maxFramesInFlight; ++i)
            overlays.add(new OverlayType(*this));

        //==============================================================================
        // Swapchain Images : There can be more swapchain framebuffers than "images in flight" !
        auto numSwapchainFrames = static_cast<uint32_t>(swapchain.getNumImages());

        for (auto i = 0U; i < numSwapchainFrames; ++i)
            swapchainFrames.add(new SwapchainFrame(swapchain, i, renderPasses.swapchain));

        //==============================================================================
        PW_DBG_V("Created render context.");
    }

    ~RenderContext() 
    { 
        // Before we can destroy all objects, the processing of all frames and command buffers must be executed   
        device.waitIdle();

        PW_DBG_V("Destroyed render context."); 
    }

    DrawStatus drawFrame(std::function<void(FrameType& frame)> drawComponents = nullptr)
    {
        // return DrawStatus::hasFinished;

        if (!swapchain.getHandle())
        {
            jassertfalse;
            return DrawStatus::hasFailed;
        }
           
        const auto renderIndex = static_cast<int>(currentFrameIndex);       
        const auto& imageCompletedFence = *imageCompletedFences[renderIndex];

        // If the frame is still in flight, wait for the corresponding render fence
        if (! imageCompletedFence.wait()) // TODO : fence wait timeout ?
        {
            //jassertfalse;
            return DrawStatus::hasFailed; 
        }
            
        if (! imageCompletedFence.reset())
        {
            jassertfalse;
            return DrawStatus::hasFailed; 
        }
            
        uint32_t swapchainImageIndex = 0;

        const auto& imageAcquiredSemaphore = *imageAcquiredSemaphores[renderIndex];

        //==============================================================================
        // Acquire swapchain framebuffer image
        {
            const auto result = swapchain.acquireNextImage(swapchainImageIndex, imageAcquiredSemaphore); // TODO : aquire timeout ?
            switch (result)
            {
                case vk::Result::eSuccess:
                    break;
                case vk::Result::eSuboptimalKHR:
                case vk::Result::eErrorOutOfDateKHR:
                    return DrawStatus::needsSwapChainRecreation;
                default:
                    jassertfalse;
                    return DrawStatus::hasFailed;
            }
        }

        //==============================================================================
        auto& frame = *frames[renderIndex];

        // Render Components to FrameBuffer
        {
            frame.reset();
            
            // Initially, the queue submit of the framebuffer commands waits for the image acquired semaphore of the swapchain
            const auto& waitSemaphore = imageAcquiredSemaphore;
            frame.setWaitSemaphore(&waitSemaphore);

            frame.beginRender();

            // Component Drawing
            if (drawComponents != nullptr)
                drawComponents(frame);

            frame.endRender();

            const auto result = frame.submit();
            if (result != vk::Result::eSuccess)
            {
                jassertfalse;
                return DrawStatus::hasFailed;
            }
        }

        //==============================================================================
        auto& overlay = *overlays[renderIndex];
        
        // Draw Offscreen Framebuffer into Swapchain Framebuffer
        {
            // The overlay queue submit waits for the framebuffer completed semaphore of the component rendering ..
            const auto& waitSemaphore = frame.getCompletedSemaphore();

            // .. and renders into the swapchain framebuffer
            const auto& swapchainFrame = *swapchainFrames[static_cast<int>(swapchainImageIndex)];

            overlay.beginRender(swapchainFrame);
            overlay.render(frame.getAttachment().imageView);
            overlay.endRender();

            const auto result = overlay.submit(waitSemaphore, imageCompletedFence);
            if (result != vk::Result::eSuccess)
            {
                jassertfalse;
                return DrawStatus::hasFailed;
            }    
        }

        //==============================================================================
        // Present swapchain framebuffer image
        {
            const auto result = swapchain.presentImage(swapchainImageIndex, overlay.getCompletedSemaphore());
            switch (result)
            {
                case vk::Result::eSuccess:
                    break;
                case vk::Result::eSuboptimalKHR:
                case vk::Result::eErrorOutOfDateKHR:
                    PW_DBG_V("Swap chain out of date/suboptimal/window resized - recreating!");
                    return DrawStatus::needsSwapChainRecreation;
                default:
                {
                    PW_DBG_V("Failed to present swap chain image!");
                    jassertfalse;
                    return DrawStatus::hasFailed;
                }
            }
        }

        //==============================================================================
        // Complete Frame
        
        incrementFrameIndex();

        ++frameCounter;

        return DrawStatus::hasFinished;
    }

private:
    void incrementFrameIndex()
    {
        currentFrameIndex = (currentFrameIndex + 1) % maxFramesInFlight;
    }

private:
    const VulkanSwapchain& swapchain;

    juce::OwnedArray<VulkanSemaphore> imageAcquiredSemaphores;
    juce::OwnedArray<VulkanFence> imageCompletedFences;
    
    juce::OwnedArray<FrameType> frames;
    juce::OwnedArray<OverlayType> overlays;

    juce::OwnedArray<SwapchainFrame> swapchainFrames;
    
    size_t currentFrameIndex = 0;
    uint64_t frameCounter = 0;
   
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RenderContext)
};

} // namespace parawave