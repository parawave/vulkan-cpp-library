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
/** Gets the image of a swapchain via index and creates a view and framebuffer. */
struct SwapchainFrame
{
    SwapchainFrame() = delete;

    SwapchainFrame(const VulkanSwapchain& sc, uint32_t swapchainImageIndex, const VulkanRenderPass& renderPass) :
        swapchainImage(sc, swapchainImageIndex), swapchainImageView(sc.getDevice(), swapchainImage),
        framebuffer(sc.getDevice(), renderPass, swapchainImageView, sc.getWidth(), sc.getHeight()) { }
    
    const VulkanImage swapchainImage;
    const VulkanImageView swapchainImageView;

    const VulkanFramebuffer framebuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SwapchainFrame)
};

//==============================================================================
/** Draws a fullscreen quad into a swapchain framebuffer using an image view. */
struct OverlayState
{
private:
    enum
    {
        numVertices = 4
    };

    using VertexType = PixelVertex;
    
public:
    OverlayState(const DeviceState& deviceState) :
        state(deviceState),
        commandBuffer(state.device), completedSemaphore(state.device),
        vertices(state.memory.vertexPool, VulkanMemoryBuffer::CreateInfo()
            .setSize<VertexType>(numVertices).setHostVisible().setVertexBuffer()),
        sampler(state.device),
        descriptor(state.images.getImageSamplerDescriptorPool()) {}

    ~OverlayState() = default;

    // Gets signaled if rendering is completed
    const VulkanSemaphore& getCompletedSemaphore() const noexcept { return completedSemaphore; }

    void beginRender(const SwapchainFrame& frame) noexcept
    {
        commandBuffer.reset();
        commandBuffer.begin();

        const auto renderBounds = frame.swapchainImage.getBounds();
        
        commandBuffer.beginRenderPass(state.renderPasses.swapchain, frame.framebuffer, renderBounds, juce::Colours::grey);

        commandBuffer.setViewport(renderBounds);
        commandBuffer.setScissor(renderBounds);

        bounds = VulkanConversion::toRectangle(renderBounds);
    }

    void render(const VulkanImageView& framebufferView)
    {
        descriptor.update(framebufferView, sampler);

        commandBuffer.bindGraphicsPipeline(state.pipelines.overlay.pipeline);
        commandBuffer.bindDescriptorSet(state.pipelines.overlay.pipelineLayout, descriptor.getDescriptorSet());
                
        const auto screenBounds = bounds.toFloat();
        
        setParameters(screenBounds.getWidth(), screenBounds.getHeight(), screenBounds);
        setVertices(screenBounds);

        commandBuffer.bindVertexBuffer(vertices.getBuffer());

        commandBuffer.draw(numVertices);
    }

    void endRender() noexcept
    {
        commandBuffer.endRenderPass();

        commandBuffer.end();
    }

    vk::Result submit(const VulkanSemaphore& waitSemaphore, const VulkanFence& completedFence) const noexcept
    {
        auto submitInfo = VulkanCommandSequence::SingleWaitSignalSubmit(commandBuffer);

        submitInfo.setWaitSemaphore(waitSemaphore);
        submitInfo.setSignalSemaphore(completedSemaphore);
        
        const auto result = state.device.getGraphicsQueue().submit(submitInfo, completedFence.getHandle());

        return result;
    }

private:
    void setParameters(float screenWidth, float screenHeight, const juce::Rectangle<float>& imageBounds)
    {
        using Parameters = OverlayPushConstants;

        Parameters values;
        values.set(screenWidth, screenHeight, imageBounds, false);

        commandBuffer.pushVertexConstants(state.pipelines.overlay.pipelineLayout, &values, sizeof(Parameters));
    }

    void setVertices(const juce::Rectangle<float>& area)
    {
        auto* v = vertexData;

        const auto left   = static_cast<int16_t>(area.getX());
        const auto top    = static_cast<int16_t>(area.getY());
        const auto right  = static_cast<int16_t>(area.getRight());
        const auto bottom = static_cast<int16_t>(area.getBottom());

        v[0].x = left;
        v[0].y = bottom;

        v[1].x = right;
        v[1].y = bottom;

        v[2].x = left;
        v[2].y = top;

        v[3].x = right;
        v[3].y = top;

        vertices.write(v, static_cast<vk::DeviceSize>(sizeof(vertexData)));
    }

private:
    const DeviceState& state;

    VulkanCommandBuffer commandBuffer;
    VulkanSemaphore completedSemaphore; 

    VulkanMemoryBuffer vertices;
    VertexType vertexData[numVertices];

    VulkanSampler sampler;

    SingleImageSamplerDescriptor descriptor;

    juce::Rectangle<int> bounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OverlayState)
};
    
} // namespace parawave