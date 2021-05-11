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
    VulkanCommandBuffer
 
    Command buffers are objects used to record commands which can be 
    subsequently submitted to a device queue for execution.
*/
class VulkanCommandBuffer final
{
private:
    VulkanCommandBuffer() = delete;

public:
    VulkanCommandBuffer(const VulkanDevice& device, const vk::CommandBufferAllocateInfo& allocateInfo)
    {
        jassert(device.getHandle());
        auto resultValue = device.getHandle().allocateCommandBuffersUnique(allocateInfo);

        PW_CHECK_VK_RESULT_SUCCESS(resultValue.result, "Couldn't create command pool.");
    
        if (resultValue.result == vk::Result::eSuccess)
            handle = std::move(resultValue.value[0]);
    }
    
    VulkanCommandBuffer(const VulkanDevice& device, const VulkanCommandPool& commandPool)
        : VulkanCommandBuffer(device, vk::CommandBufferAllocateInfo()
        .setCommandPool(commandPool.getHandle()).setLevel(vk::CommandBufferLevel::ePrimary).setCommandBufferCount(1)) {}

    VulkanCommandBuffer(const VulkanDevice& device)
        : VulkanCommandBuffer(device, device.getGraphicsCommandPool()) {}
    
    ~VulkanCommandBuffer() = default;

    const vk::CommandBuffer& getHandle() const noexcept { return *handle; }

    void reset() const noexcept;

    void begin(vk::CommandBufferUsageFlagBits usage = vk::CommandBufferUsageFlagBits::eOneTimeSubmit) const noexcept;

    void end() const noexcept;

    //==============================================================================

    void beginRenderPass(const VulkanRenderPass& renderPass, const VulkanFramebuffer& framebuffer, const vk::Rect2D& renderArea, const juce::Colour& clearColour = juce::Colours::transparentBlack) const noexcept;

    void endRenderPass() const noexcept;

    void clearColour(const vk::Rect2D& clearArea, const juce::Colour& clearColour = juce::Colours::transparentBlack) const noexcept;

    //==============================================================================

    void setViewport(const vk::Viewport& viewport) const noexcept;

    void setViewport(const vk::Rect2D& bounds) const noexcept;

    void setScissor(const vk::Rect2D& bounds) const noexcept;

    //==============================================================================

    void bindPipeline(const VulkanPipeline& pipeline, vk::PipelineBindPoint bindPoint) const noexcept;

    void bindGraphicsPipeline(const VulkanPipeline& pipeline) const noexcept;

    void bindComputePipeline(const VulkanPipeline& pipeline) const noexcept;

    void bindDescriptorSet(const VulkanPipelineLayout& pipelineLayout, const VulkanDescriptorSet& descriptorSet) const noexcept;

    void bindVertexBuffer(const VulkanBuffer& vertexBuffer) const noexcept;

    void bindIndexBuffer(const VulkanBuffer& indexBuffer, vk::IndexType indexType = vk::IndexType::eUint16) const noexcept;

    //==============================================================================
    
    void pushConstants(const VulkanPipelineLayout& layout, const void* constantData, uint32_t dataSize, uint32_t dataOffset = 0, vk::ShaderStageFlags stageFlags = vk::ShaderStageFlagBits::eAllGraphics) const noexcept;

    void pushVertexConstants(const VulkanPipelineLayout& layout, const void* constantData, uint32_t dataSize, uint32_t dataOffset = 0) const noexcept;

    void pushFragmentConstants(const VulkanPipelineLayout& layout, const void* constantData, uint32_t dataSize, uint32_t dataOffset = 0) const noexcept;

    //==============================================================================

    void draw(uint32_t numTriangles, uint32_t firstVertex = 0) const noexcept;

    void drawIndexed(uint32_t numIndices, uint32_t firstIndex = 0, int32_t vertexOffset = 0) const noexcept;

    void drawIndexedInstanced(uint32_t numInstances, uint32_t numIndices, uint32_t instanceOffset = 0, uint32_t firstIndex = 0, int32_t vertexOffset = 0) const noexcept;
    
    void dispatchCompute(uint32_t groupCountX = 1, uint32_t groupCountY = 1, uint32_t groupCountZ = 1) const noexcept;

    //==============================================================================

    void copyBuffer(const VulkanBuffer& dest, const VulkanBuffer& src, const vk::BufferCopy& region) const noexcept;

    void copyBufferToImage(const VulkanImage& dest, const VulkanBuffer& src, const vk::BufferImageCopy& region, vk::ImageLayout dstImageLayout = vk::ImageLayout::eTransferDstOptimal) const noexcept;

    void copyImageToBuffer(const VulkanBuffer& dest, const VulkanImage& src, const vk::BufferImageCopy& region, vk::ImageLayout srcImageLayout = vk::ImageLayout::eTransferSrcOptimal) const noexcept;

    void transitionImageLayout(const VulkanImage& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const noexcept;

    //==============================================================================

    /** Use a lamda to record and submit commands into the default graphics queue.
        This will immediately submit and execute commands and uses a waitIdle() for syncronisation. 
        
        Due to the queue wait idle, it's probably a good idea to avoid this for normal render code 
        and only use it for coarse initialisation commands.
    */
    template <typename CommandsFunction>
    static void submit(const VulkanDevice& device, const CommandsFunction& commandsFunction) noexcept
    {
        const VulkanCommandBuffer commandBuffer(device, device.getGraphicsCommandPool());
        submit(device.getGraphicsQueue(), commandBuffer, commandsFunction);
    }

    template <typename CommandsFunction>
    static void submit(const VulkanDevice::Queue& queue, const VulkanCommandBuffer& commandBuffer, const CommandsFunction& commandsFunction) noexcept
    {
        commandBuffer.begin();

        jassert(commandBuffer.getHandle());
        commandsFunction(commandBuffer);

        commandBuffer.end();

        queue.submit(commandBuffer);
        queue.waitIdle();    
    }

private:
    vk::UniqueCommandBuffer handle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanCommandBuffer)
};

} // namespace parawave