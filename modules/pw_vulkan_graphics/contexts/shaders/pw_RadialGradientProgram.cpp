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
    
//==============================================================================
struct RadialGradientPushConstants
{
    VulkanUniform::ScreenBounds screenBounds;
    VulkanUniform::Matrix matrix;

    void set2DBounds (const juce::Rectangle<float>& bounds) noexcept
    {
        screenBounds.set(bounds);
    }

    void setMatrix(const juce::Point<float>& p1, const juce::Point<float>& p2, const juce::Point<float>& p3) noexcept
    {
        auto t = juce::AffineTransform::fromTargetPoints (p1, juce::Point<float>(),
                                                          p2, juce::Point<float> (1.0f, 0.0f),
                                                          p3, juce::Point<float> (0.0f, 1.0f));
        matrix.set(t);
    }
};

//==============================================================================
class RadialGradientProgram
{
private:
    struct PipelineLayoutInfo : public vk::PipelineLayoutCreateInfo
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
            vk::PushConstantRange(vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(RadialGradientPushConstants))
        };
    };

    struct PipelineInfo : public ProgramHelpers::GraphicsPipelineCreateInfo
    {
        PipelineInfo(VulkanDevice& device, const VulkanPipelineLayout& pipelineLayout, const VulkanRenderPass& renderPass)
            : ProgramHelpers::GraphicsPipelineCreateInfo(pipelineLayout, renderPass)
        {
            setShaders(device, "RadialGradient.vert", "RadialGradient.frag");
            finish();
        }
    };

public:
    RadialGradientProgram(VulkanDevice& device, const VulkanDescriptorSetLayout& descriptorSetLayout, const VulkanRenderPass& renderPass) :
        pipelineLayout(device, PipelineLayoutInfo(descriptorSetLayout)),
        pipeline(device, PipelineInfo(device, pipelineLayout, renderPass)) { }

    const VulkanPipelineLayout pipelineLayout;
    const VulkanPipeline pipeline;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RadialGradientProgram)
};

} // namespace parawave