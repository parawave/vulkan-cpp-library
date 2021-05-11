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
    VulkanCommandPool
 
    Command pools are opaque objects that command buffer memory is allocated 
    from, and which allow the implementation to amortize the cost of resource 
    creation across multiple command buffers. Command pools are externally 
    synchronized, meaning that a command pool must not be used concurrently in 
    multiple threads. That includes use via recording commands on any command 
    buffers allocated from the pool, as well as operations that allocate, free, 
    and reset command buffers or the pool itself.
*/
class VulkanCommandPool final
{
private:
    VulkanCommandPool() = delete;

public:
    VulkanCommandPool(const VulkanDevice& device, const vk::CommandPoolCreateInfo& createInfo)
    {
        vk::Result result;

        jassert(device.getHandle());
        std::tie(result, handle) = device.getHandle().createCommandPoolUnique(createInfo).asTuple();

        PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't create command pool.");
    }

    VulkanCommandPool(const VulkanDevice& device, uint32_t queueFamilyIndex, vk::CommandPoolCreateFlagBits flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        : VulkanCommandPool(device, vk::CommandPoolCreateInfo().setQueueFamilyIndex(queueFamilyIndex).setFlags(flags)) {}
    
    ~VulkanCommandPool() = default;

    const vk::CommandPool& getHandle() const noexcept { return *handle; }

private:
    vk::UniqueCommandPool handle;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanCommandPool)
};

} // namespace parawave