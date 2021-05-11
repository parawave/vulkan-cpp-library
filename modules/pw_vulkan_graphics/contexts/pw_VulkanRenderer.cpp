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

#pragma once

namespace parawave
{

//==============================================================================
/** 
    VulkanRenderer
    
    Acquire access to parts of the Vulkan LowLevelGraphicsContext 
    implementation. 
 
    This interface is not meant for public access!

    BUT ... It could be used in other modules to setup own graphics pipelines.

    void paint(juce::Graphics& g)
    {
        if(auto renderer = VulkanRenderer::get(g))
        {
            ... setup custom render pipeline ...

            renderer->restoreRenderState();
        }
    }
*/
class VulkanRenderer
{
public:
    VulkanRenderer() = default;
    virtual ~VulkanRenderer() = default;

    /** Get the associated device the renderer uses for its resources. */
    virtual VulkanDevice& getDevice() noexcept = 0;

    /** Get the command buffer the current renderer is using.*/
    virtual const VulkanCommandBuffer& getCommandBuffer() const noexcept = 0;

    /** Get the renderpass the current renderer is using.*/
    virtual const VulkanRenderPass& getRenderPass() const noexcept = 0;

    /** Get the current render framebuffer bounds. */
    virtual juce::Rectangle<int> getRenderBounds() const noexcept = 0;

    /** Get the current render transform. */
    virtual juce::AffineTransform getRenderTransform() const noexcept = 0;

    /** Get a shader module that was previously loaded with 'loadShaderModule' */
    virtual const VulkanShaderModule* getShaderModule(const char* name) const = 0;

    /** Load a Vulkan SPIRV shader module from SPV bytecode (uint32). */
    virtual void loadShaderModule(const char* name, const void* spvData, const int dataSize) = 0;

    virtual VulkanMemoryPool& getVertexMemoryPool() const noexcept = 0;

    virtual const VulkanDescriptorSetLayout& getTextureDescriptorLayout() const noexcept = 0;

    virtual const VulkanDescriptorSet& getTextureDescriptorSet(const VulkanTexture& texture, juce::Graphics::ResamplingQuality quality) const = 0;

    virtual VulkanTexture::Ptr getTextureFor(const juce::Image& image) const = 0;

    //==============================================================================
    class Listener
    {
    public:
        virtual ~Listener() = default;

        /** Gets called before the renderer is destroyed. */
        virtual void rendererClosing(VulkanRenderer& target) = 0;
    };

    virtual void addListener(Listener* listenerToAdd) = 0;
    virtual void removeListener(Listener* listenerToRemove) = 0;

    //==============================================================================

    virtual void restoreRenderState() = 0;

    /** If a VulkanContext is attached to a juce::Component, the Component::paint(juce::Graphics); will
        be called with a Vulkan LowLevelGraphicsContext. Use this method to get the corresponding 
        VulkanRenderer and access the VulkanDevice to setup new render methods. */
    static VulkanRenderer* get(const juce::Graphics& g) noexcept;
};
    
} // namespace parawave