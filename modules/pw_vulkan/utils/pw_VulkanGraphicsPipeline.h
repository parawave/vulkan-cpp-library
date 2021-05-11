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
class VulkanGraphicsPipeline final
{
public:
    //==============================================================================
    /** 
        A preinitialised CreateInfo struct to simply the GraphicsPipeline creation.
        Use the helper methods to setup common parameters and call finish() to 
        complete the CreateInfo.
    */
    struct CreateInfo : public vk::GraphicsPipelineCreateInfo
    {
    public:
        CreateInfo(const VulkanPipelineLayout& pipelineLayout, const VulkanRenderPass& renderPass)
        {
            setLayout(pipelineLayout.getHandle());
            setRenderPass(renderPass.getHandle());

            inputAssemblyState
                .setTopology(vk::PrimitiveTopology::eTriangleList);

            viewportState
                .setViewports(defaultViewport)
                .setScissors(defaultScissor);

            rasterizationState
                .setDepthClampEnable(false)
                .setRasterizerDiscardEnable(false)
                .setPolygonMode(vk::PolygonMode::eFill)
                .setCullMode(vk::CullModeFlagBits::eNone)
                .setFrontFace(vk::FrontFace::eClockwise)
                .setDepthBiasEnable(false)
                .setDepthBiasConstantFactor(0.0f)
                .setDepthBiasClamp(0.0f)
                .setDepthBiasSlopeFactor(0.0f)
                .setLineWidth(1.0f);

            multisampleState
                .setRasterizationSamples(vk::SampleCountFlagBits::e1);

            setAlphaBlending(defaultBlendAttachmentState);

            colorBlendState
                .setLogicOpEnable(false)
                .setLogicOp(vk::LogicOp::eNoOp)
                .setAttachments(defaultBlendAttachmentState)
                .setBlendConstants({ 1.0f, 1.0f, 1.0f, 1.0f });

            defaultDynamicStates = 
            {
                vk::DynamicState::eViewport,
                vk::DynamicState::eScissor
            };

            dynamicState
                .setDynamicStates(defaultDynamicStates);
        }

        void finish()
        {
            setStages(shaderStages);
            setPVertexInputState(&vertexInputState);
            setPInputAssemblyState(&inputAssemblyState);
            setPTessellationState(nullptr);
            setPViewportState(&viewportState);
            setPRasterizationState(&rasterizationState);
            setPMultisampleState(&multisampleState);
            setPDepthStencilState(nullptr);
            setPColorBlendState(&colorBlendState);
            setPDynamicState(&dynamicState);
        }

        void setShaderStages(const VulkanShaderModule& vertShader, const VulkanShaderModule& fragShader)
        {
            jassert(vertShader.getHandle() && fragShader.getHandle());
        
            shaderStages = 
            {
                vk::PipelineShaderStageCreateInfo( vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertShader.getHandle(), "main"),
                vk::PipelineShaderStageCreateInfo( vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fragShader.getHandle(), "main")
            };
        }

        void setAlphaBlending(vk::PipelineColorBlendAttachmentState& blendState)
        {
            blendState
                .setBlendEnable(true)
                .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
                .setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
                .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                .setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                .setColorBlendOp(vk::BlendOp::eAdd)
                .setAlphaBlendOp(vk::BlendOp::eAdd)
                .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
        }

        void setPremultipliedAlphaBlending(vk::PipelineColorBlendAttachmentState& blendState)
        {
            blendState
                .setBlendEnable(true)
                .setSrcColorBlendFactor(vk::BlendFactor::eOne)
                .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                .setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                .setColorBlendOp(vk::BlendOp::eAdd)
                .setAlphaBlendOp(vk::BlendOp::eAdd)
                .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
        }

        /*
            (sR*sA) + (dR*dA) = rR 	
            (sG*sA) + (dG*dA) = rG
            (sB*sA) + (dB*dA) = rB
            (sA*1)  + (dA*1)  = rA
        */
        void setAlphaAccumulationBlending(vk::PipelineColorBlendAttachmentState& blendState)
        {
            blendState
                .setBlendEnable(true)
                .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
                .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                .setDstColorBlendFactor(vk::BlendFactor::eDstAlpha)
                .setDstAlphaBlendFactor(vk::BlendFactor::eOne)
                .setColorBlendOp(vk::BlendOp::eAdd)
                .setAlphaBlendOp(vk::BlendOp::eAdd)
                .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
        }

        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
   
        vk::PipelineVertexInputStateCreateInfo vertexInputState;
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
        vk::PipelineViewportStateCreateInfo viewportState;
        vk::PipelineRasterizationStateCreateInfo rasterizationState;
        vk::PipelineMultisampleStateCreateInfo multisampleState;
        vk::PipelineColorBlendStateCreateInfo colorBlendState;
        vk::PipelineDynamicStateCreateInfo dynamicState;

    private:
        vk::Viewport defaultViewport;
        vk::Rect2D defaultScissor;

        vk::PipelineColorBlendAttachmentState defaultBlendAttachmentState;

        std::vector<vk::DynamicState> defaultDynamicStates;
    };
};

} // namespace parawave