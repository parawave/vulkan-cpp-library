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
class VulkanDescriptor final
{
private:
    VulkanDescriptor() = delete;

public:
    VulkanDescriptor(VulkanDescriptorSetPool& pool_) : pool(pool_)
    {
        std::tie(block, descriptorSet) = pool.acquire();
        jassert(block && descriptorSet);
    }

    ~VulkanDescriptor()
    {
        jassert(block && descriptorSet);
        block->dispose(descriptorSet);
    }

    const VulkanDescriptorSet& getDescriptorSet() const noexcept { return *descriptorSet; }

    void updateDescriptorSet(const vk::WriteDescriptorSet* descriptorWrites, uint32_t descriptorWriteCount, const vk::CopyDescriptorSet* descriptorCopies = nullptr, uint32_t descriptorCopyCount = 0) const noexcept
    {
        const auto& device = pool.getDevice();
        jassert(device.getHandle());
        
        device.getHandle().updateDescriptorSets(descriptorWriteCount, descriptorWrites, descriptorCopyCount, descriptorCopies);
    }

private:
    VulkanDescriptorSetPool& pool;

    VulkanDescriptorSetPool::Block* block;
    const VulkanDescriptorSet* descriptorSet;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanDescriptor)
};
    
} // namespace parawave