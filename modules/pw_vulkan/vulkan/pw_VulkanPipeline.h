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
/** Represents a Vulkan pipeline. */
class VulkanPipeline final
{
private:
    VulkanPipeline() = delete;

public:
    /** Graphics pipelines consist of multiple shader stages, multiple 
        fixed-function pipeline stages, and a pipeline layout. */ 
    VulkanPipeline(const VulkanDevice& device, const vk::GraphicsPipelineCreateInfo& createInfo)
    {
        vk::Result result;

        jassert(device.getHandle());
        std::tie(result, handle) = device.getHandle().createGraphicsPipelineUnique(nullptr, createInfo).asTuple();
        
        PW_CHECK_VK_RESULT(result == vk::Result::eSuccess || result == vk::Result::ePipelineCompileRequiredEXT, result, "Couldn't create graphics pipeline.");

        if (result == vk::Result::ePipelineCompileRequiredEXT)
        {
            /** TODO : PipelineCompileRequiredEXT ? */
            jassertfalse;
        }
    }
    //==============================================================================
    /** Compute pipelines consist of a single static compute shader stage and the 
        pipeline layout. */
    VulkanPipeline(const VulkanDevice& device, const vk::ComputePipelineCreateInfo& createInfo)
    {
        vk::Result result;

        jassert(device.getHandle());
        std::tie(result, handle) = device.getHandle().createComputePipelineUnique(nullptr, createInfo).asTuple();

        PW_CHECK_VK_RESULT(result == vk::Result::eSuccess || result == vk::Result::ePipelineCompileRequiredEXT, result, "Couldn't create compute pipeline.");

        if (result == vk::Result::ePipelineCompileRequiredEXT)
        {
            /** TODO : PipelineCompileRequiredEXT ? */
            jassertfalse;
        }
    }
    
    ~VulkanPipeline() = default;

    const vk::Pipeline& getHandle() const noexcept { return *handle; }

private:
    vk::UniquePipeline handle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanPipeline)
};

} // namespace parawave