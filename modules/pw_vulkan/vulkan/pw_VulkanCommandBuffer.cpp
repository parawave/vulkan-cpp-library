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
   
namespace CommandBufferHelpers
{

struct BarrierOptions
{
    BarrierOptions(vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
    {
        setImageLayoutTransition(oldLayout, newLayout);
    }

    void setImageLayoutTransition(vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
    {
        if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) 
        {
            srcAccessMask = vk::AccessFlagBits();
            dstAccessMask = vk::AccessFlagBits::eTransferWrite;

            srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
            dstStage = vk::PipelineStageFlagBits::eTransfer;
        } 
        else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) 
        {
            srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            dstAccessMask = vk::AccessFlagBits::eShaderRead;

            srcStage = vk::PipelineStageFlagBits::eTransfer;
            dstStage = vk::PipelineStageFlagBits::eFragmentShader;
        } 
        else if (oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal && newLayout == vk::ImageLayout::eTransferSrcOptimal)
        {
            srcAccessMask = vk::AccessFlagBits::eShaderRead;
            dstAccessMask = vk::AccessFlagBits::eTransferRead;

            srcStage = vk::PipelineStageFlagBits::eFragmentShader;
            dstStage = vk::PipelineStageFlagBits::eTransfer;
        }
        else if (oldLayout == vk::ImageLayout::eTransferSrcOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
        {
            srcAccessMask = vk::AccessFlagBits::eTransferRead;
            dstAccessMask = vk::AccessFlagBits::eShaderRead;

            srcStage = vk::PipelineStageFlagBits::eTransfer;
            dstStage = vk::PipelineStageFlagBits::eFragmentShader;
        }
        else 
        {
            PW_DBG_V("Unsupported layout transition!");
            jassertfalse;
        }
    }

    vk::AccessFlags srcAccessMask;
    vk::AccessFlags dstAccessMask;
    
    vk::PipelineStageFlags srcStage;
    vk::PipelineStageFlags dstStage;
};

} // namespace CommandBufferHelpers

//==============================================================================

void VulkanCommandBuffer::reset() const noexcept
{
    jassert(handle);
    handle->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
}

void VulkanCommandBuffer::begin(vk::CommandBufferUsageFlagBits usage) const noexcept
{
    const auto beginInfo = vk::CommandBufferBeginInfo().setFlags(usage);

    jassert(handle);
    const auto result = handle->begin(beginInfo);

    PW_CHECK_VK_RESULT_SUCCESS(result, "Failed to begin command buffer recording.");
}
    
void VulkanCommandBuffer::end() const noexcept
{
    jassert(handle);
    const auto result = handle->end();
        
    PW_CHECK_VK_RESULT_SUCCESS(result, "Failed Failed to end command buffer recording.");
}

void VulkanCommandBuffer::beginRenderPass(const VulkanRenderPass& renderPass, const VulkanFramebuffer& framebuffer, const vk::Rect2D& renderArea, const juce::Colour& clearColour) const noexcept
{
    auto clearValue = vk::ClearValue()
        .setColor(VulkanConversion::toClearColorValue(clearColour));

    auto renderPassInfo = vk::RenderPassBeginInfo()
        .setRenderPass(renderPass.getHandle())
        .setFramebuffer(framebuffer.getHandle())
        .setRenderArea(renderArea)
        .setClearValues(clearValue);

    handle->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
}

void VulkanCommandBuffer::endRenderPass() const noexcept
{
    handle->endRenderPass();
}

void VulkanCommandBuffer::clearColour(const vk::Rect2D& clearArea, const juce::Colour& clearColour) const noexcept
{
    auto clearValue = vk::ClearValue()
        .setColor(VulkanConversion::toClearColorValue(clearColour));

    std::array<vk::ClearAttachment, 1> clearAttachments;

    clearAttachments[0]
        .setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setClearValue(clearValue)
        .setColorAttachment(0);

    std::array<vk::ClearRect, 1> clearRects;

    clearRects[0]
        .setBaseArrayLayer(0)
        .setLayerCount(1)
        .setRect(clearArea);
  
    handle->clearAttachments(clearAttachments, clearRects);
}

void VulkanCommandBuffer::setViewport(const vk::Viewport& viewport) const noexcept
{
    handle->setViewport(0, 1, &viewport);
}

void VulkanCommandBuffer::setViewport(const vk::Rect2D& bounds) const noexcept
{
    setViewport(VulkanConversion::toViewport(bounds));
}

void VulkanCommandBuffer::setScissor(const vk::Rect2D& bounds) const noexcept
{
    handle->setScissor(0, 1, &bounds);
}

void VulkanCommandBuffer::bindPipeline(const VulkanPipeline& pipeline, vk::PipelineBindPoint bindPoint) const noexcept
{
    handle->bindPipeline(bindPoint, pipeline.getHandle());
}

void VulkanCommandBuffer::bindGraphicsPipeline(const VulkanPipeline& pipeline) const noexcept
{
    bindPipeline(pipeline, vk::PipelineBindPoint::eGraphics);
}

void VulkanCommandBuffer::bindComputePipeline(const VulkanPipeline& pipeline) const noexcept
{
    bindPipeline(pipeline, vk::PipelineBindPoint::eCompute);
}

void VulkanCommandBuffer::bindDescriptorSet(const VulkanPipelineLayout& pipelineLayout, const VulkanDescriptorSet& descriptorSet) const noexcept
{
    handle->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout.getHandle(), 0, 1, &descriptorSet.getHandle(), 0, nullptr);
}

void VulkanCommandBuffer::bindVertexBuffer(const VulkanBuffer& vertexBuffer) const noexcept
{
    vk::Buffer vertexBuffers[] = { vertexBuffer.getHandle() };
    vk::DeviceSize offsets[] = { 0 };
            
    handle->bindVertexBuffers(0, 1, vertexBuffers, offsets);
}

void VulkanCommandBuffer::bindIndexBuffer(const VulkanBuffer& indexBuffer, vk::IndexType indexType) const noexcept
{
    handle->bindIndexBuffer(indexBuffer.getHandle(), 0, indexType);
}

void VulkanCommandBuffer::pushConstants(const VulkanPipelineLayout& layout, const void* constantData, uint32_t dataSize, uint32_t dataOffset, vk::ShaderStageFlags stageFlags) const noexcept
{
    jassert(handle && layout.getHandle());
    handle->pushConstants(layout.getHandle(), stageFlags, dataOffset, dataSize, constantData);
}

void VulkanCommandBuffer::pushVertexConstants(const VulkanPipelineLayout& layout, const void* constantData, uint32_t dataSize, uint32_t dataOffset) const noexcept
{
    pushConstants(layout, constantData, dataSize, dataOffset, vk::ShaderStageFlagBits::eVertex);
}

void VulkanCommandBuffer::pushFragmentConstants(const VulkanPipelineLayout& layout, const void* constantData, uint32_t dataSize, uint32_t dataOffset) const noexcept
{
    pushConstants(layout, constantData, dataSize, dataOffset, vk::ShaderStageFlagBits::eFragment);
}

void VulkanCommandBuffer::draw(uint32_t numTriangles, uint32_t firstVertex) const noexcept
{
    handle->draw(numTriangles, 1, firstVertex, 0);
}

void VulkanCommandBuffer::drawIndexed(uint32_t numIndices, uint32_t firstIndex, int32_t vertexOffset) const noexcept
{
    handle->drawIndexed(numIndices, 1, firstIndex, vertexOffset, 0);
}

void VulkanCommandBuffer::drawIndexedInstanced(uint32_t numInstances, uint32_t numIndices, uint32_t instanceOffset, uint32_t firstIndex, int32_t vertexOffset) const noexcept
{
    handle->drawIndexed(numIndices, numInstances, firstIndex, vertexOffset, instanceOffset);
}

void VulkanCommandBuffer::dispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const noexcept
{
    handle->dispatch(groupCountX, groupCountY, groupCountZ);
}

void VulkanCommandBuffer::copyBuffer(const VulkanBuffer& dest, const VulkanBuffer& src, const vk::BufferCopy& region) const noexcept
{
    handle->copyBuffer(src.getHandle(), dest.getHandle(), region);
}

void VulkanCommandBuffer::copyBufferToImage(const VulkanImage& dest, const VulkanBuffer& src, const vk::BufferImageCopy& region, vk::ImageLayout dstImageLayout) const noexcept
{
    handle->copyBufferToImage(src.getHandle(), dest.getHandle(), dstImageLayout, 1, &region);
}

void VulkanCommandBuffer::copyImageToBuffer(const VulkanBuffer& dest, const VulkanImage& src, const vk::BufferImageCopy& region, vk::ImageLayout srcImageLayout) const noexcept
{
    handle->copyImageToBuffer(src.getHandle(), srcImageLayout, dest.getHandle(), 1, &region);
}

void VulkanCommandBuffer::transitionImageLayout(const VulkanImage& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const noexcept
{
    const auto imageSubresource = vk::ImageSubresourceRange()
        .setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setBaseMipLevel(0)
        .setLevelCount(1)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    const CommandBufferHelpers::BarrierOptions options(oldLayout, newLayout);
        
    const auto barrier = vk::ImageMemoryBarrier()
        .setSrcAccessMask(options.srcAccessMask)
        .setDstAccessMask(options.dstAccessMask)
        .setOldLayout(oldLayout)
        .setNewLayout(newLayout)
        .setSrcQueueFamilyIndex({})
        .setDstQueueFamilyIndex({})
        .setImage(image.getHandle())
        .setSubresourceRange(imageSubresource);

    handle->pipelineBarrier(options.srcStage, options.dstStage, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &barrier);
}

} // namespace parawave