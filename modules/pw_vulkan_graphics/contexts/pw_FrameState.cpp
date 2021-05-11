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
class FrameState : public RenderLayer
{
public:
    FrameState(DeviceState& deviceState, uint32_t width, uint32_t height, vk::Format format) :
        RenderLayer(deviceState, width, height, format), renderCache(std::make_unique<RenderCache>(deviceState))
    {
        setCache(renderCache.get());
    }

    /** Initialize cached frame data from the last frame before a new render pass is started. */
    void reset()
    {
        renderCache->reset();
        quadQueue.reset();
    }

private:
    std::unique_ptr<RenderCache> renderCache;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrameState)
};

//==============================================================================
/** Holds a frame state and can be used to immediately render to it using a fence at the end as sync. */
class ImmediateFrameState
{
public:
    ImmediateFrameState(const VulkanContext& context_, uint32_t width, uint32_t height, bool shouldClearImage, vk::Format renderFormat) :
        context(context_), 
        deviceState(*context.getDevice(), renderFormat),
        frame(deviceState, width, height, renderFormat),
        fence(deviceState.device), renderClearFlag(shouldClearImage) {}

    ~ImmediateFrameState()
    {
        deviceState.setMinimizeStorageOnRelease(false);

        /** If the Immediate State is deleted, but the fence not completed, we still have to wait
            for it to complete. */
        if ( !fenceCompleted)
        {
            fence.waitIdle();
        }
    }

    void startRender()
    {
        if (fence.wait())
        {
            // Immediate rendering doesn't wait for previous submits and will not signal !
            frame.setWaitSemaphore(nullptr);
            frame.setSignalSemaphore(nullptr);

            frame.reset();
            frame.beginRender(renderClearFlag);

            renderStartedFlag = true;
            renderClearFlag = false;
        }
        else
        {
            PW_DBG_V("[Vulkan] Couldn't wait for render fence. The FrameState is still in use.");
            jassertfalse;
        }
    }

    void flushRender()
    {
        if (! renderStartedFlag)
            return;

        frame.endRender();

        if (! fence.reset())
            return;

        fenceCompleted = false;

        const auto result = frame.submit(fence);
        if (result == vk::Result::eSuccess)
        {
            if (fence.wait())
            {
                fenceCompleted = true;
            }
            else
            {
                // jassertfalse; 

                /** 
                    Rendering couldn't complete in time!
                 
                    TODO: Not sure what's the best thing to do here if fence couldn't wait.

                    For now: Just device wait idle to be sure everything completed!
                */
                deviceState.device.waitIdle();
            }
        }
        else
        {
            deviceState.device.waitIdle();

            // Invalid queue submit
            jassertfalse;
        }

        renderStartedFlag = false;
    }

    const VulkanContext& context;
    DeviceState deviceState;

    FrameState frame;
    VulkanFence fence;

    bool renderStartedFlag = false;
    bool renderClearFlag = false;

    bool fenceCompleted = true;
};

} // namespace parawave