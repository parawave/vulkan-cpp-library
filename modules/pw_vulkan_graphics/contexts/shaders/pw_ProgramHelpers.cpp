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
    
namespace ProgramHelpers
{

//==============================================================================
struct Vertex
{
    uint16_t x;
    uint16_t y;
    uint32_t colour;
};

//==============================================================================
struct GraphicsPipelineCreateInfo : public VulkanGraphicsPipeline::CreateInfo
{
    struct PushConstants
    {
        VulkanUniform::ScreenBounds screenBounds;
    };

    GraphicsPipelineCreateInfo(const VulkanPipelineLayout& pipelineLayout, const VulkanRenderPass& renderPass)
        : VulkanGraphicsPipeline::CreateInfo(pipelineLayout, renderPass)
    {
        vertexInputState
                .setVertexBindingDescriptions(bindings)
                .setVertexAttributeDescriptions(attributes);

        setPremultipliedAlphaBlending(blendAttachmentState);

        colorBlendState
            .setAttachments(blendAttachmentState);
    }

    void setShaders(VulkanDevice& device, const char* vertShaderName, const char* fragShaderName)
    {
        CachedShaders::Ptr shaders = CachedShaders::get(device);

        const auto vertShader = shaders->getShaderModule(vertShaderName);
        const auto fragShader = shaders->getShaderModule(fragShaderName);

        jassert(vertShader != nullptr && fragShader != nullptr);
        
        setShaderStages(*vertShader, *fragShader);
    }

    vk::PipelineColorBlendAttachmentState blendAttachmentState;

    std::array<vk::VertexInputBindingDescription, 1> bindings =
    {
        vk::VertexInputBindingDescription(0, sizeof(Vertex), vk::VertexInputRate::eVertex)
    };

    std::array<vk::VertexInputAttributeDescription, 2> attributes
    {
        vk::VertexInputAttributeDescription(0, 0, vk::Format::eR16G16Sscaled, offsetof(Vertex, x)),
        vk::VertexInputAttributeDescription(1, 0, vk::Format::eA8B8G8R8UnormPack32, offsetof(Vertex, colour)) // eB8G8R8A8Unorm
    };
};

//==============================================================================
struct PipelineLayoutInfo : vk::PipelineLayoutCreateInfo
{
    PipelineLayoutInfo()
    {
        setPushConstantRanges(pushConstantRanges);
    }

    std::array<vk::PushConstantRange, 1> pushConstantRanges =
    {
        vk::PushConstantRange(vk::ShaderStageFlagBits::eVertex, 0, sizeof(GraphicsPipelineCreateInfo::PushConstants))
    };
};

//==============================================================================
struct ComputePipelineCreateInfo : public VulkanComputePipeline::CreateInfo
{
    ComputePipelineCreateInfo(const VulkanPipelineLayout& pipelineLayout) :
        VulkanComputePipeline::CreateInfo(pipelineLayout) {}

    void setShader(VulkanDevice& device, const char* computeShaderName)
    {
        CachedShaders::Ptr shaders = CachedShaders::get(device);

        const auto computeShader = shaders->getShaderModule(computeShaderName);
        jassert(computeShader != nullptr);
        
        setShaderStage(*computeShader);
    }
};

} // namespace ProgramHelpers

} // namespace parawave