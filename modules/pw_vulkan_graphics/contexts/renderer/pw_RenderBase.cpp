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
/** Holds wait/signal semaphores, completed fence and command buffer. */
class RenderBase : public VulkanRenderer
{
public:
    RenderBase(const DeviceState& deviceState) : 
        state(deviceState), commandBuffer(state.device), completedSemaphore(state.device)
    { 
        setSignalSemaphore(&completedSemaphore);
    }

    ~RenderBase() override = default;

    const DeviceState& getDeviceState() const noexcept { return state; }

    const VulkanSemaphore& getCompletedSemaphore() const noexcept { return completedSemaphore; }

    const VulkanSemaphore* getWaitSemaphore() const noexcept { return currentWaitSemaphore; }

    void setWaitSemaphore(const VulkanSemaphore* newSemaphore) noexcept { currentWaitSemaphore = newSemaphore; }

    const VulkanSemaphore* getSignalSemaphore() const noexcept { return currentSignalSemaphore; }

    void setSignalSemaphore(const VulkanSemaphore* newSemaphore) noexcept { currentSignalSemaphore = newSemaphore; }

    vk::Result submit() const noexcept
    {
        auto submitInfo = VulkanCommandSequence::SingleWaitSignalSubmit(commandBuffer);

        if (currentWaitSemaphore != nullptr)
            submitInfo.setWaitSemaphore(*currentWaitSemaphore);

        if (currentSignalSemaphore != nullptr)
            submitInfo.setSignalSemaphore(*currentSignalSemaphore);

        return state.device.getGraphicsQueue().submit(submitInfo);
    }

    vk::Result submit(const VulkanFence& fence) const noexcept
    {
        auto submitInfo = VulkanCommandSequence::SingleWaitSignalSubmit(commandBuffer);

        if (currentWaitSemaphore != nullptr)
            submitInfo.setWaitSemaphore(*currentWaitSemaphore);

        if (currentSignalSemaphore != nullptr)
            submitInfo.setSignalSemaphore(*currentSignalSemaphore);

        return state.device.getGraphicsQueue().submit(submitInfo, fence.getHandle());
    }

    //==============================================================================
    // implements : VulkanRenderer

    VulkanDevice& getDevice() noexcept override { return state.getDevice(); }

    const VulkanCommandBuffer& getCommandBuffer() const noexcept override { return commandBuffer; }

    const VulkanRenderPass& getRenderPass() const noexcept override { return state.renderPasses.offscreen; }

    const VulkanShaderModule* getShaderModule(const char* name) const override
    {
        return state.shaders.getShaderModule(name);
    }

    void loadShaderModule(const char* name, const void* spvData, const int dataSize) override
    {
        state.shaders.loadModule(name, spvData, dataSize);
    }

    void addListener(Listener* listenerToAdd) override { listenerList.add(listenerToAdd); }
    void removeListener(Listener* listenerToRemove) override { listenerList.remove(listenerToRemove); }

    VulkanMemoryPool& getVertexMemoryPool() const noexcept override
    {
        return state.memory.vertexPool;
    }

    const VulkanDescriptorSetLayout& getTextureDescriptorLayout() const noexcept override
    {
        return state.images.getImageSamplerDescriptorPool().layout;
    }

    const VulkanDescriptorSet& getTextureDescriptorSet(const VulkanTexture& texture, juce::Graphics::ResamplingQuality quality) const override
    {
        auto descriptor = state.images.getTextureDescriptor(texture, quality);
        return descriptor->getDescriptorSet();
    }

    VulkanTexture::Ptr getTextureFor(const juce::Image& image) const override
    {
        return state.images.getTextureFor(image);
    }

protected:
    void notifyClose()
    {
        listenerList.call([&](Listener& l) { l.rendererClosing(*this); });
    }

protected:
    const DeviceState& state;
    const VulkanCommandBuffer commandBuffer;
    
private:
    const VulkanSemaphore completedSemaphore;

    const VulkanSemaphore* currentWaitSemaphore = nullptr;
    const VulkanSemaphore* currentSignalSemaphore = nullptr;

    juce::ListenerList<Listener> listenerList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RenderBase)
};

} // namespace parawave