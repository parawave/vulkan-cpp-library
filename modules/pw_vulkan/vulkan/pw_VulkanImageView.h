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
    VulkanImageView

    Image objects are not directly accessed by pipeline shaders for reading or 
    writing image data. Instead, image views representing contiguous ranges of 
    the image subresources and containing additional metadata are used for that 
    purpose. Views must be created on images of compatible types, and must 
    represent a valid subset of image subresources.
*/
class VulkanImageView final
{
public:
    struct CreateInfo : public vk::ImageViewCreateInfo
    {
        CreateInfo(const vk::Image& image, const vk::Format& imageFormat)
        {
            vk::ComponentMapping componentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA);
            vk::ImageSubresourceRange subResourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

            setImage(image);
            setViewType(vk::ImageViewType::e2D);
            setFormat(imageFormat);
            setComponents(componentMapping);
            setSubresourceRange(subResourceRange);
        }
    };

public:
    VulkanImageView(const VulkanDevice& device, const vk::ImageViewCreateInfo& createInfo)
    {
        vk::Result result;
    
        jassert(device.getHandle());
        std::tie(result, handle) = device.getHandle().createImageViewUnique(createInfo).asTuple();

        PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't create image view.");
    }

    VulkanImageView(const VulkanDevice& device, const VulkanImage& image)
        : VulkanImageView(device, CreateInfo(image.getHandle(), image.getFormat())) {}

    ~VulkanImageView() = default;

    const vk::ImageView& getHandle() const noexcept { return *handle; }

private:
    vk::UniqueImageView handle;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanImageView)
};

} // namespace parawave