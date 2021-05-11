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
    VulkanFence

    Fences are a synchronization primitive that can be used to insert a 
    dependency from a queue to the host. Fences have two states - signaled and 
    unsignaled.

    A fence can be signaled as part of the execution of a queue submission 
    command.

    Fences can be unsignaled on the host with reset(). 
    Fences can be waited on by the host with wait().
*/
class VulkanFence final
{
private:
    VulkanFence() = delete;

public:
    VulkanFence(const VulkanDevice& device_, const vk::FenceCreateInfo& createInfo) : device(device_)
    {
        vk::Result result;

        jassert(device.getHandle());
        std::tie(result, handle) = device.getHandle().createFenceUnique(createInfo).asTuple();

        PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't create fence.");
    }

    VulkanFence(const VulkanDevice& device)
        : VulkanFence(device, vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled)) {}

    ~VulkanFence() = default;

    const vk::Fence& getHandle() const noexcept { return *handle; }

    bool isSignaled() const noexcept
    {
        jassert(getHandle() && device.getHandle());
        const auto result = device.getHandle().getFenceStatus(getHandle());

        PW_CHECK_VK_RESULT(result == vk::Result::eSuccess || result == vk::Result::eNotReady, result, "Couldn't get fence status.");

        return result == vk::Result::eSuccess;
    }

    bool wait(juce::RelativeTime duration = juce::RelativeTime::seconds(1.0)) const noexcept
    {
        const auto timeout = static_cast<uint64_t>(std::max<int64_t>(0, duration.inMilliseconds())) * 1000; // Nano Seconds

        jassert(getHandle() && device.getHandle());
        const auto result = device.getHandle().waitForFences(1, &getHandle(), VK_TRUE, timeout);

        PW_CHECK_VK_RESULT(result == vk::Result::eSuccess || result == vk::Result::eTimeout, result, "Couldn't wait for fence.");

        return result == vk::Result::eSuccess;
    }

    /** Idles in a sleep loop with the specified duration until the fence wait completes. */
    void waitIdle(juce::RelativeTime duration = juce::RelativeTime::milliseconds(10)) const noexcept
    {
        const auto halfDuration = static_cast<int>(duration.seconds(duration.inSeconds() / 2.0).inMilliseconds());

        while (! wait(duration))
            juce::Thread::sleep(halfDuration);
    }

    bool reset() const noexcept
    {
        jassert(getHandle() && device.getHandle());
        const auto result = device.getHandle().resetFences(1, &getHandle());

        PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't reset fence.");

        return result == vk::Result::eSuccess;
    }

private:
    const VulkanDevice& device;
    vk::UniqueFence handle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanFence)
};
    
} // namespace parawave