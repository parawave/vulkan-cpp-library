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
    VulkanBuffer

    Buffers represent linear arrays of data which are used for various purposes 
    by binding them to a graphics or compute pipeline via descriptor sets or via 
    certain commands, or by directly specifying them as parameters to certain 
    commands.
*/
class VulkanBuffer final
{
private:
    VulkanBuffer() = delete;

public:
    VulkanBuffer(const VulkanDevice& device_, const vk::BufferCreateInfo& createInfo)
        : device(device_), size(createInfo.size) 
    { 
        vk::Result result;
    
        jassert(device.getHandle());
        std::tie(result, handle) = device.getHandle().createBufferUnique(createInfo).asTuple();

        PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't create buffer.");
    }
    
    VulkanBuffer(const VulkanDevice& device, vk::DeviceSize bufferSize, vk::BufferUsageFlags bufferUsage, vk::SharingMode sharingMode = vk::SharingMode::eExclusive)
        : VulkanBuffer(device, vk::BufferCreateInfo().setSize(bufferSize).setUsage(bufferUsage).setSharingMode(sharingMode)) {}

    ~VulkanBuffer() = default;

    const vk::Buffer& getHandle() const noexcept { return *handle; }

    vk::DeviceSize getSize() const noexcept { return size; }

    vk::MemoryRequirements getMemoryRequirements() const noexcept
    {
        jassert(getHandle() && device.getHandle());
        return device.getHandle().getBufferMemoryRequirements(getHandle());
    }

private:
    const VulkanDevice& device;

    vk::UniqueBuffer handle;
    vk::DeviceSize size;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanBuffer)
};
    
} // namespace parawave#pragma once
