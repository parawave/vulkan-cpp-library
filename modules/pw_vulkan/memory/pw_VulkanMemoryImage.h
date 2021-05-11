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
    VulkanMemoryImage
*/
class VulkanMemoryImage final
{
private:
    VulkanMemoryImage() = delete;

public:
    struct CreateInfo
    {
        CreateInfo(uint32_t width_ = {}, uint32_t height_ = {}, vk::Format imageFormat_ = {}, vk::ImageUsageFlagBits imageUsage_ = {}, vk::MemoryPropertyFlags memoryProperties_ = {}) : 
            width(width_), height(height_), imageFormat(imageFormat_), imageUsage(imageUsage_), memoryProperties(memoryProperties_) {}

        CreateInfo& setWidth(uint32_t width_) noexcept { width = width_; return *this; }

        CreateInfo& setHeight(uint32_t height_) noexcept { height = height_; return *this; }

        CreateInfo& setFormat(vk::Format imageFormat_) noexcept { imageFormat = imageFormat_; return *this; }

        CreateInfo& setUsage(vk::ImageUsageFlagBits imageUsage_) noexcept { imageUsage = imageUsage_; return *this; }

        CreateInfo& setSampled() noexcept { imageUsage |= vk::ImageUsageFlagBits::eSampled; return *this; }

        CreateInfo& setColorAttachment() noexcept { imageUsage |= vk::ImageUsageFlagBits::eColorAttachment; return *this; }
        
        CreateInfo& setTransferDst() noexcept { imageUsage |= vk::ImageUsageFlagBits::eTransferDst; return *this; }

        CreateInfo& setTransferSrc() noexcept { imageUsage |= vk::ImageUsageFlagBits::eTransferSrc; return *this; }

        CreateInfo& setMemoryProperties(vk::MemoryPropertyFlags memoryProperties_) noexcept { memoryProperties = memoryProperties_; return *this; }

        CreateInfo& setHostVisible() noexcept
        {
            memoryProperties |= (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent); return *this;
        }

        CreateInfo& setDeviceLocal() noexcept
        {
            memoryProperties |= vk::MemoryPropertyFlagBits::eDeviceLocal; return *this;
        }

        uint32_t width = {};
        uint32_t height = {};
        vk::Format imageFormat = {};
        vk::ImageUsageFlags imageUsage = {};
        vk::MemoryPropertyFlags memoryProperties = {};
    };

public:
    VulkanMemoryImage(VulkanMemoryPool& pool_, const vk::ImageCreateInfo& imageCreateInfo, vk::MemoryPropertyFlags memoryProperties) : 
        pool(pool_), image(pool_.getDevice(), imageCreateInfo), memoryRange(pool_.acquire(image.getMemoryRequirements(), memoryProperties))
    { 
        if (auto memoryBlock = memoryRange.getMemoryBlock())
        {
            const auto& device = pool.getDevice();

            const auto& deviceMemory = memoryBlock->getDeviceMemory();
            jassert(deviceMemory.getHandle());

            // jassert(memoryRange.getSize() >= buffer.getSize()); TODO: image.getSize() ? 

            jassert(device.getHandle() && image.getHandle());
            auto result = device.getHandle().bindImageMemory(image.getHandle(), deviceMemory.getHandle(), memoryRange.getOffset());

            PW_CHECK_VK_RESULT_SUCCESS(result, "Failed to bind device memory for image.");
        }
        else
        {
            jassertfalse;
        }
    }

    VulkanMemoryImage(VulkanMemoryPool& pool, uint32_t width, uint32_t height, 
        vk::Format imageFormat, vk::ImageUsageFlags imageUsage, vk::MemoryPropertyFlags memoryProperties) : 
        VulkanMemoryImage(pool, VulkanImage::CreateInfo(width, height, imageFormat, imageUsage), memoryProperties) {}

    VulkanMemoryImage(VulkanMemoryPool& pool_, CreateInfo createInfo) :
        VulkanMemoryImage(pool_, createInfo.width, createInfo.height, createInfo.imageFormat, createInfo.imageUsage, createInfo.memoryProperties) {}

    ~VulkanMemoryImage()
    {
        pool.dispose(memoryRange, defragmentFlag);
    }

    const VulkanImage& getImage() const noexcept { return image; }

    vk::DeviceSize getMemorySize() const noexcept { return memoryRange.getSize(); }

    bool useDefragmentOnRelease() const noexcept { return defragmentFlag; }
    void setDefragmentOnRelease(bool defragmentOnRelease) noexcept { defragmentFlag = defragmentOnRelease; }

    bool isHostVisible() const noexcept { return getData() != nullptr; }

    // Get the host visible memory address of the requested range. Will return nullpt if it's unmapped.
    void* getData() const noexcept 
    { 
        if(const auto memoryBlock = memoryRange.getMemoryBlock())
            if (memoryBlock->isHostVisible())
                return reinterpret_cast<uint8_t*>(memoryBlock->getData()) + memoryRange.getOffset();

        return nullptr;
    }

private:
    VulkanMemoryPool& pool;

    const VulkanImage image;
    const VulkanMemoryRange memoryRange;

    bool defragmentFlag = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanMemoryImage)
};

} // namespace parawave