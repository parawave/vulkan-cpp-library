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
    VulkanMemoryPool 

    A pool of DeviceMemory allocations. 
*/
class VulkanMemoryPool final
{
public:
    VulkanMemoryPool(const VulkanDevice& device_, vk::DeviceSize minBlockSize_) : 
        device(device_), minBlockSize(minBlockSize_) {}

    const VulkanDevice& getDevice() const noexcept { return device; }

    vk::DeviceSize size() const noexcept 
    { 
        vk::DeviceSize totalSize = 0;

        for (auto& block : blocks)
            totalSize += block->size();

        return totalSize; 
    }

    VulkanMemoryRange acquire(vk::DeviceSize requiredSize, vk::DeviceSize requiredAlignment, uint32_t memoryTypeIndex)
    {
        VulkanMemoryRange range;

        for (auto& block : blocks)
        {
            if (memoryTypeIndex == block->getMemoryTypeIndex())
                if (block->acquireRange(range, requiredSize, requiredAlignment))
                    return range;
        }

        auto newBlock = blocks.add(allocateBlock(requiredSize, memoryTypeIndex).release());

        const auto acquired = newBlock->acquireRange(range, requiredSize, requiredAlignment);
        jassert(acquired);
        
        return range;
    }
    
    VulkanMemoryRange acquire(const vk::MemoryRequirements& memoryRequirements, vk::MemoryPropertyFlags memoryProperties)
    {
        const auto memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, memoryProperties);
        return acquire(memoryRequirements.size, memoryRequirements.alignment, memoryTypeIndex);
    }

    void dispose(const VulkanMemoryRange& range, bool performDefragmentation = true)
    {
        for (auto block : blocks)
        {
            if (block->contains(range))
            {
                block->disposeRange(range, performDefragmentation);

                return;
            }
        }

        // Range was not acquired by this allocator or is already disposed
        jassertfalse;
    }

    void minimizeStorage()
    {
        deallocateEmptyBlocks();
    }

private:
    static vk::DeviceSize nextPowerOfTwo(vk::DeviceSize size) noexcept
    {
        const auto power = static_cast<vk::DeviceSize>(std::log2l(static_cast<long double>(size))) + 1;
        return static_cast<vk::DeviceSize>(1) << power;
    }

    std::unique_ptr<VulkanMemory> allocateBlock(vk::DeviceSize allocationSize, uint32_t memoryType) const noexcept
    {
        allocationSize = (allocationSize > minBlockSize) ? nextPowerOfTwo(allocationSize) : minBlockSize;
        return std::make_unique<VulkanMemory>(device, allocationSize, memoryType);
    }

    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const noexcept
    {
        const auto& memProperties = device.getPhysicalDevice().getMemoryProperties();
        for (auto i = 0U; i < memProperties.memoryTypeCount; ++i)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                return i;
        }

        PW_DBG_V("Failed to find suitable memory type.");
        jassertfalse;

        return 0U;
    }

    void deallocateEmptyBlocks()
    {
        for (int i = blocks.size(); --i >= 0;)
        {
            if (blocks[i]->isFree())
                blocks.remove(i, true);
        }
    }

private:
    const VulkanDevice& device;
    const vk::DeviceSize minBlockSize;

    juce::OwnedArray<VulkanMemory> blocks;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanMemoryPool)
};

} // namespace parawave