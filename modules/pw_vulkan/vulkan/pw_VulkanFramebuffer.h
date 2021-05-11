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
    VulkanFramebuffer

    Render passes operate in conjunction with framebuffers. Framebuffers 
    represent a collection of specific memory attachments that a render pass 
    instance uses.
*/
class VulkanFramebuffer final
{
private:
    VulkanFramebuffer() = delete;

public:
    struct CreateInfo : public vk::FramebufferCreateInfo
    {
        CreateInfo(const VulkanRenderPass& renderPass, const VulkanImageView& imageView, uint32_t width, uint32_t height)
        {
            attachments[0] = imageView.getHandle();

            setRenderPass(renderPass.getHandle());
            setAttachments(attachments);
            setWidth(width);
            setHeight(height);
            setLayers(1);
        }

        std::array<vk::ImageView, 1> attachments;
    };

public:
    VulkanFramebuffer(const VulkanDevice& device, const vk::FramebufferCreateInfo& createInfo)
    {
        vk::Result result;

        jassert(device.getHandle());
        std::tie(result, handle) = device.getHandle().createFramebufferUnique(createInfo).asTuple();

        PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't create framebuffer");
    }

    VulkanFramebuffer(const VulkanDevice& device, const VulkanRenderPass& renderPass, const VulkanImageView& imageView, uint32_t width, uint32_t height) :
        VulkanFramebuffer(device, CreateInfo(renderPass, imageView, width, height)) {}

    ~VulkanFramebuffer() = default;

    const vk::Framebuffer& getHandle() const noexcept { return *handle; }

private:
    vk::UniqueFramebuffer handle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanFramebuffer)
};

} // namespace parawave