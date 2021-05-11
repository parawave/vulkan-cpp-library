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
    VulkanDescriptorSet

    Descriptors are grouped together into descriptor set objects. A descriptor 
    set object is an opaque object containing storage for a set of descriptors, 
    where the types and number of descriptors is defined by a descriptor set 
    layout.

    Descriptor sets are allocated from descriptor pool objects.
*/
class VulkanDescriptorSet final
{
private:
    VulkanDescriptorSet() = delete;

public:
    VulkanDescriptorSet(const VulkanDevice& device, const vk::DescriptorSetAllocateInfo& allocateInfo)
    {
        auto resultValue = device.getHandle().allocateDescriptorSetsUnique(allocateInfo);

        PW_CHECK_VK_RESULT_SUCCESS(resultValue.result, "Couldn't create descriptor set.");

        if (resultValue.result == vk::Result::eSuccess)
            handle = std::move(resultValue.value[0]);
    }

    VulkanDescriptorSet(const VulkanDevice& device, const VulkanDescriptorPool& descriptorPool, const VulkanDescriptorSetLayout& descriptorSetLayout)
        : VulkanDescriptorSet(device, vk::DescriptorSetAllocateInfo()
            .setDescriptorPool(descriptorPool.getHandle())
            .setDescriptorSetCount(1)
            .setPSetLayouts(&descriptorSetLayout.getHandle())) {}

    ~VulkanDescriptorSet() = default;

    const vk::DescriptorSet& getHandle() const noexcept { return *handle; }

private:
    vk::UniqueDescriptorSet handle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanDescriptorSet)
};

} // namespace parawave