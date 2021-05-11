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
    VulkanDeviceMemory

    Device memory is memory that is visible to the device — for example the 
    contents of the image or buffer objects, which can be natively used by the 
    device.

    Memory properties of a physical device describe the memory heaps and memory 
    types available.
*/
class VulkanDeviceMemory final
{
private:
    VulkanDeviceMemory() = delete;

public:
    VulkanDeviceMemory(const VulkanDevice& device, const vk::MemoryAllocateInfo& allocateInfo)
    {
        vk::Result result;

        jassert(device.getHandle());
        std::tie(result, handle) = device.getHandle().allocateMemoryUnique(allocateInfo).asTuple();

        PW_CHECK_VK_RESULT_SUCCESS(result, "Failed to allocate device memory.");
    }
    
    ~VulkanDeviceMemory() = default;

    const vk::DeviceMemory& getHandle() const noexcept { return *handle; }

private:
    vk::UniqueDeviceMemory handle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanDeviceMemory)
};
    
} // namespace parawave
