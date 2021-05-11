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
    A shader specifies programmable operations that execute for each vertex, 
    control point, tessellated vertex, primitive, fragment, or workgroup in the 
    corresponding stage(s) of the graphics and compute pipelines.

    Shader modules contain shader code and one or more entry points. Shaders 
    are selected from a shader module by specifying an entry point as part of 
    pipeline creation. The stages of a pipeline can use shaders that come from 
    different modules. The shader code defining a shader module must be in the 
    SPIR-V format
*/
class VulkanShaderModule final
{
private:
    VulkanShaderModule() = delete;

public:
    VulkanShaderModule(const VulkanDevice& device, const vk::ShaderModuleCreateInfo& createInfo)
    {
        vk::Result result;

        jassert(device.getHandle());
        std::tie(result, handle) = device.getHandle().createShaderModuleUnique(createInfo).asTuple();

        PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't create shader module.");
    }

    VulkanShaderModule(const VulkanDevice& device, const uint32_t* spvData, size_t dataSize)
        : VulkanShaderModule(device, vk::ShaderModuleCreateInfo().setCodeSize(dataSize).setPCode(spvData)) {}

    VulkanShaderModule(const VulkanDevice& device, const juce::MemoryBlock& shaderSPV)
        : VulkanShaderModule(device, reinterpret_cast<const uint32_t*>(shaderSPV.getData()), shaderSPV.getSize()) {}

    ~VulkanShaderModule() = default;

    const vk::ShaderModule& getHandle() const noexcept { return *handle; }

private:
    vk::UniqueShaderModule handle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanShaderModule)
};

} // namespace parawave