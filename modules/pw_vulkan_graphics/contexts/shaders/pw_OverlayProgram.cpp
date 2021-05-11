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
  
struct PixelVertex
{
    int16_t x;
    int16_t y;
};

//==============================================================================
struct OverlayPushConstants
{
    juce::Point<float> screenSize;
    float textureBounds[4];
    juce::Point<float> vOffsetAndScale;

    void set(const float targetWidth, const float targetHeight, const juce::Rectangle<float>& bounds, bool flipVertically)
    {
        screenSize.setXY(targetWidth, targetHeight);

        textureBounds[0] = bounds.getX();
        textureBounds[1] = bounds.getY();
        textureBounds[2] = bounds.getWidth();
        textureBounds[3] = bounds.getHeight();

        vOffsetAndScale.setXY(flipVertically ? 1.0f : 0.0f, flipVertically ? -1.0f : 1.0f);
    }
};

//==============================================================================
class OverlayProgram
{
private:
    struct PipelineLayoutInfo : vk::PipelineLayoutCreateInfo
    {
        PipelineLayoutInfo(const VulkanDescriptorSetLayout& descriptorSetLayout)
        {
            descriptorSetLayouts[0] = descriptorSetLayout.getHandle();

            setSetLayouts(descriptorSetLayouts);
            setPushConstantRanges(pushConstantRanges);
        }

        std::array<vk::DescriptorSetLayout, 1> descriptorSetLayouts;

        std::array<vk::PushConstantRange, 1> pushConstantRanges =
        {
            vk::PushConstantRange(vk::ShaderStageFlagBits::eVertex, 0, sizeof(OverlayPushConstants))
        };
    };
    
    struct PipelineInfo : public ProgramHelpers::GraphicsPipelineCreateInfo
    {
        PipelineInfo(VulkanDevice& device, const VulkanPipelineLayout& pipelineLayout, const VulkanRenderPass& renderPass)
            : ProgramHelpers::GraphicsPipelineCreateInfo(pipelineLayout, renderPass)
        {
            setShaders(device, "Overlay.vert", "Overlay.frag");

            vertexInputState
                .setVertexBindingDescriptions(bindings)
                .setVertexAttributeDescriptions(attributes);

            inputAssemblyState.setTopology(vk::PrimitiveTopology::eTriangleStrip);

            finish();
        }

        std::array<vk::VertexInputBindingDescription, 1> bindings
        {
            vk::VertexInputBindingDescription(0, sizeof(PixelVertex), vk::VertexInputRate::eVertex)
        };

        std::array<vk::VertexInputAttributeDescription, 1> attributes
        {
            vk::VertexInputAttributeDescription(0, 0, vk::Format::eR16G16Sscaled, offsetof(PixelVertex, x))
        };
    };

public:
    OverlayProgram(VulkanDevice& device, const VulkanDescriptorSetLayout& descriptorSetLayout, const VulkanRenderPass& renderPass) :
        pipelineLayout(device, PipelineLayoutInfo(descriptorSetLayout)),
        pipeline(device, PipelineInfo(device, pipelineLayout, renderPass)) {}

    const VulkanPipelineLayout pipelineLayout;
    const VulkanPipeline pipeline;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OverlayProgram)
};

} // namespace parawave