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
    VulkanImage
 
    Images represent multidimensional - up to 3 - arrays of data which can be 
    used for various purposes (e.g. attachments, textures), by binding them to a 
    graphics or compute pipeline via descriptor sets, or by directly specifying 
    them as parameters to certain commands.
*/
class VulkanImage final
{
public:
    struct CreateInfo : public vk::ImageCreateInfo
    {
        CreateInfo(uint32_t width, uint32_t height, vk::Format imageFormat, vk::ImageUsageFlags imageUsage)
        {
            setImageType(vk::ImageType::e2D);
            setFormat(imageFormat);
            setExtent(vk::Extent3D(width, height, 1));
            setMipLevels(1);
            setArrayLayers(1);
            setSamples(vk::SampleCountFlagBits::e1);
            setTiling(vk::ImageTiling::eOptimal);
            setUsage(imageUsage);
            setSharingMode(vk::SharingMode::eExclusive);
            setQueueFamilyIndices(queueFamilyIndices);
            setInitialLayout(vk::ImageLayout::eUndefined);
        }

        std::array<uint32_t, 0> queueFamilyIndices;
    };

public:
    VulkanImage(const VulkanDevice& device_, const vk::ImageCreateInfo& createInfo)
        : device(device_), extent(createInfo.extent), format(createInfo.format)
    {
        vk::Result result;

        jassert(device.getHandle());
        std::tie(result, handle) = device.getHandle().createImageUnique(createInfo).asTuple();

        PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't create image.");
    }

    VulkanImage(const VulkanDevice& device, uint32_t width, uint32_t height, vk::Format format, vk::ImageUsageFlags usage)
        : VulkanImage(device, CreateInfo(width, height, format, usage)) { }

    VulkanImage(const VulkanSwapchain& swapchain, uint32_t swapChainImageIndex) 
        : device(swapchain.getDevice()), swapchainImage(swapchain.getImage(swapChainImageIndex)), 
          extent(swapchain.getWidth(), swapchain.getHeight(), 1),
          format(swapchain.getImageFormat()) {}

    ~VulkanImage() = default;

    const vk::Image& getHandle() const noexcept  { return handle ? *handle : swapchainImage; }

    const vk::Extent3D& getExtent() const noexcept { return extent; }

    vk::Rect2D getBounds() const noexcept { return { vk::Offset2D(), vk::Extent2D(extent.width, extent.height) }; }

    uint32_t getWidth() const noexcept { return extent.width; }

    uint32_t getHeight() const noexcept { return extent.height; }

    const vk::Format& getFormat() const noexcept  { return format; }

    vk::MemoryRequirements getMemoryRequirements() const noexcept
    {
        jassert(getHandle() && device.getHandle());
        return device.getHandle().getImageMemoryRequirements(getHandle());
    }

private:
    const VulkanDevice& device;

    vk::Image swapchainImage;
    vk::UniqueImage handle;
    
    vk::Extent3D extent;
    vk::Format format;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanImage)
};

} // namespace parawave