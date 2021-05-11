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
    VulkanDescriptorSetPool 

    Holds and creates multiple descriptor pools with pre-allocated descriptor 
    sets. 
*/
class VulkanDescriptorSetPool final
{
    friend class VulkanDescriptor;

private:
    struct Block final
    {
    public:
        Block(const VulkanDevice& device, const VulkanDescriptorSetLayout& descriptorSetLayout, const vk::DescriptorPoolCreateInfo& createInfo) :
            pool(device, createInfo)
        {
            const auto numSets = createInfo.maxSets;

            sets.ensureStorageAllocated(static_cast<int>(numSets));
            free.ensureStorageAllocated(static_cast<int>(numSets));
            
            for (auto i = 0U; i < numSets; ++i)
                sets.add(new VulkanDescriptorSet(device, pool, descriptorSetLayout));

            for (auto& set : sets)
                free.add(set);

            PW_DBG_V("Created descriptor pool with " << juce::String(numSets) << " sets");
        }

        ~Block()
        {
            PW_DBG_V("Deleted descriptor pool with (" << juce::String(free.size()) << " free, " << juce::String(sets.size()) << " used) sets");
        }

        bool isEmpty() const noexcept { return free.isEmpty(); }

        const VulkanDescriptorSet* acquire() noexcept
        {
            return free.removeAndReturn(free.size() - 1);
        }

        void dispose(const VulkanDescriptorSet* descriptorSet) noexcept
        {
            free.add(descriptorSet);
        }

    private:
        VulkanDescriptorPool pool;

        juce::OwnedArray<const VulkanDescriptorSet> sets;
        juce::Array<const VulkanDescriptorSet*> free;
    
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Block)
    };

public:
    VulkanDescriptorSetPool(const VulkanDevice& device_, const VulkanDescriptorSetLayout& descriptorSetLayout_, const vk::DescriptorPoolCreateInfo& createInfo_) :
        device(device_), descriptorSetLayout(descriptorSetLayout_), createInfo(createInfo_) 
    {
        // Create at least one initial block
        blocks.add(new Block(device, descriptorSetLayout, createInfo));
    }

    const VulkanDevice& getDevice() const noexcept { return device; }

    const VulkanDescriptorSetLayout& getDescriptorSetLayout() const noexcept { return descriptorSetLayout; }

private:
    std::pair<Block*, const VulkanDescriptorSet*> acquire()
    {
        for (auto block : blocks)
        {
            if (!block->isEmpty())
                return std::make_pair(block, block->acquire());
        }

        auto newBlock = blocks.add(new Block(device, descriptorSetLayout, createInfo));
        return std::make_pair(newBlock, newBlock->acquire());
    }

private:
    const VulkanDevice& device;
    const VulkanDescriptorSetLayout& descriptorSetLayout;

    const vk::DescriptorPoolCreateInfo& createInfo;

    juce::OwnedArray<Block> blocks;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanDescriptorSetPool)
};
    
} // namespace parawave