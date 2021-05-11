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
    VulkanPhysicalDevice
*/
class VulkanPhysicalDevice final
{
public:
    struct QueueFamily final
    {
        bool isGraphicsQueue() const noexcept { return static_cast<bool>(flags & vk::QueueFlagBits::eGraphics); }

        bool isComputeQueue() const noexcept { return static_cast<bool>(flags & vk::QueueFlagBits::eCompute); }

        uint32_t index;
        uint32_t count;
            
        vk::QueueFlags flags;
    };

private:
    VulkanPhysicalDevice() = delete;

public:
    explicit VulkanPhysicalDevice(const VulkanInstance& instance, const vk::PhysicalDevice& handle);

    ~VulkanPhysicalDevice() = default;

    const VulkanInstance& getInstance() const noexcept { return instance; }

    const vk::PhysicalDevice& getHandle() const noexcept { return handle; }

    const vk::PhysicalDeviceType& getType() const noexcept { return type; }

    const vk::PhysicalDeviceLimits& getLimits() const noexcept { return limits; }

    const vk::PhysicalDeviceMemoryProperties& getMemoryProperties() const noexcept { return memoryProperties; }

    juce::String getName() const { return name; }

    juce::String getDeviceTypeName() const { return vk::to_string(type); }

    bool isDiscreteGpu() const noexcept { return type == vk::PhysicalDeviceType::eDiscreteGpu; }

    bool isSurfaceSupported(const vk::SurfaceKHR& surface) const noexcept;

    const juce::Array<QueueFamily>& getQueueFamilies() const noexcept { return queueFamilies; }

private:
    const VulkanInstance& instance;

    vk::PhysicalDevice handle;
    vk::PhysicalDeviceType type;
    vk::PhysicalDeviceLimits limits;
    vk::PhysicalDeviceMemoryProperties memoryProperties;

    juce::String name;

    juce::Array<QueueFamily> queueFamilies;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanPhysicalDevice)
};
    
} // namespace parawave