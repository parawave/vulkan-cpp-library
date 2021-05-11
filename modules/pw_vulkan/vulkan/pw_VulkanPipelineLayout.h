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
    VulkanPipelineLayout
 
    Access to descriptor sets from a pipeline is accomplished through a pipeline 
    layout. Zero or more descriptor set layouts and zero or more push constant 
    ranges are combined to form a pipeline layout object describing the complete 
    set of resources that can be accessed by a pipeline. The pipeline layout 
    represents a sequence of descriptor sets with each having a specific layout. 
    This sequence of layouts is used to determine the interface between shader 
    stages and shader resources. Each pipeline is created using a pipeline 
    layout.
*/
class VulkanPipelineLayout final
{
private:
    VulkanPipelineLayout() = delete;

public:
    VulkanPipelineLayout(const VulkanDevice& device, const vk::PipelineLayoutCreateInfo& createInfo)
    { 
        vk::Result result;
    
        jassert(device.getHandle());
        std::tie(result, handle) = device.getHandle().createPipelineLayoutUnique(createInfo).asTuple();

        PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't create pipeline layout.");
    }

    ~VulkanPipelineLayout() = default;

    const vk::PipelineLayout& getHandle() const noexcept { return *handle; }

private:
    vk::UniquePipelineLayout handle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanPipelineLayout)
};
    
} // namespace parawave