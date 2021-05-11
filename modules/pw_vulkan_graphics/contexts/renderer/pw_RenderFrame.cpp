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
class RenderFrame : public RenderBase
{
public:
    struct Attachment
    {
        Attachment(VulkanMemoryPool& pool, const VulkanMemoryImage::CreateInfo& createInfo) : 
            memoryImage(pool, createInfo), imageView(pool.getDevice(), memoryImage.getImage()){ }
    
        const VulkanMemoryImage memoryImage;
        const VulkanImageView imageView;
    };

public:
    RenderFrame(const DeviceState& deviceState, uint32_t width, uint32_t height, vk::Format format) : 
        RenderBase(deviceState),
        attachment(state.memory.framebufferPool, VulkanMemoryImage::CreateInfo(width, height, format)
            .setDeviceLocal().setColorAttachment().setSampled().setTransferDst().setTransferSrc()),
        framebuffer(state.device, state.renderPasses.offscreen, attachment.imageView, width, height)
    {
        bounds.setSize(static_cast<int>(width), static_cast<int>(height));
    }

    ~RenderFrame() override
    {
        notifyClose();
    }

    const Attachment& getAttachment() const noexcept { return attachment; }

    juce::Rectangle<int> getBounds() const noexcept { return bounds; }

    juce::AffineTransform getTransform() const noexcept { return transformSource ? transformSource->getTransform() : juce::AffineTransform(); }

    void setTransformSource(juce::RenderingHelpers::TranslationOrTransform* newTransformSource)
    {
        transformSource = newTransformSource;
    }

    void setBounds(const juce::Rectangle<int>& newBounds) noexcept
    {
        bounds = newBounds;
    }

    void clearColour(vk::Rect2D area, juce::Colour colour = juce::Colours::transparentBlack)
    {
        commandBuffer.clearColour(area, colour);
    }

    void clearColour(juce::Colour colour = juce::Colours::transparentBlack)
    {
        auto area = attachment.memoryImage.getImage().getBounds();
        clearColour(area, colour);
    }

    void beginRender(bool clearFramebuffer = false)
    {
        // jassert(cache);

        commandBuffer.reset();
        commandBuffer.begin();

        // Begin Pass : Limit render area so it's definitely inside of the framebuffer extent
        {
            auto renderArea = attachment.memoryImage.getImage().getBounds();

            renderArea.extent.width = std::min(renderArea.extent.width, static_cast<uint32_t>(bounds.getWidth()));
            renderArea.extent.height = std::min(renderArea.extent.height, static_cast<uint32_t>(bounds.getHeight()));

            commandBuffer.beginRenderPass(state.renderPasses.offscreen, framebuffer, renderArea);
            
            if(clearFramebuffer)
                clearColour(renderArea);
        }
        
        const auto viewArea = VulkanConversion::toRect2D(bounds.withZeroOrigin());

        commandBuffer.setViewport(viewArea);
        commandBuffer.setScissor(viewArea);

        currentPipeline = nullptr;
        
        currentPipelineLayout = nullptr;
        currentDescriptorSet = nullptr;

        initialiseBindings();
    }

    void endRender()
    {
        resetBindings();

        commandBuffer.endRenderPass();
        commandBuffer.end(); 
    }

    virtual void initialiseBindings() = 0;

    virtual void resetBindings() = 0;

    void bindPipeline(const VulkanPipeline& newPipeline) noexcept
    {
        if (&newPipeline != currentPipeline)
        {
            currentPipeline = &newPipeline;
            commandBuffer.bindGraphicsPipeline(newPipeline);
        }
    }

    void bindDescriptorSet(const VulkanPipelineLayout& newPipelineLayout, const VulkanDescriptorSet& newDescriptorSet) noexcept
    {
        if (&newPipelineLayout != currentPipelineLayout || &newDescriptorSet != currentDescriptorSet)
        {
            currentPipelineLayout = &newPipelineLayout;
            currentDescriptorSet = &newDescriptorSet;

            commandBuffer.bindDescriptorSet(newPipelineLayout, newDescriptorSet);
        }
    }

    //==============================================================================
    // implements : VulkanRenderer

    juce::Rectangle<int> getRenderBounds() const noexcept override { return getBounds(); }

    juce::AffineTransform getRenderTransform() const noexcept override { return getTransform(); }

    void restoreRenderState() override
    {
        currentPipeline = nullptr;

        currentPipelineLayout = nullptr;
        currentDescriptorSet = nullptr;

        initialiseBindings();
    }

private:
    Attachment attachment;
    const VulkanFramebuffer framebuffer;

    juce::Rectangle<int> bounds;

    juce::RenderingHelpers::TranslationOrTransform* transformSource = nullptr;

    const VulkanPipeline* currentPipeline = nullptr;

    const VulkanPipelineLayout* currentPipelineLayout = nullptr;
    const VulkanDescriptorSet* currentDescriptorSet = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RenderFrame)
};

} // namespace parawave