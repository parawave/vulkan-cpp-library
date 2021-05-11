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
    VulkanSemaphore

    Semaphores are a synchronization primitive that can be used to insert a 
    dependency between queue operations or between a queue operation and the 
    host. 
*/
class VulkanSemaphore final
{
private:
    VulkanSemaphore() = delete;

public:
    VulkanSemaphore(const VulkanDevice& device, const vk::SemaphoreCreateInfo& createInfo)
    {
        vk::Result result;

        jassert(device.getHandle());
        std::tie(result, handle) = device.getHandle().createSemaphoreUnique(createInfo).asTuple();
        
        PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't create semaphore.");
    }

    VulkanSemaphore(const VulkanDevice& device)
        : VulkanSemaphore(device, vk::SemaphoreCreateInfo()) {}

    ~VulkanSemaphore() = default;

    const vk::Semaphore& getHandle() const noexcept { return *handle; }

private:
    vk::UniqueSemaphore handle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanSemaphore)
};

} // namespace parawave