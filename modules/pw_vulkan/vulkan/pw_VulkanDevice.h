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
    VulkanDevice
    
    Vulkan separates the concept of physical and logical devices. 
    A physical device usually represents a single complete implementation of 
    Vulkan (excluding instance-level functionality) available to the host, of 
    which there are a finite number. A logical device represents an instance of 
    that implementation with its own state and resources independent of other 
    logical devices.
*/
class VulkanDevice final
{
public:
    struct Queue final
    {
        const vk::Queue& getHandle() const noexcept { return handle; }

        vk::Result submit(const VulkanCommandBuffer& commandBuffer, vk::Fence fence = nullptr) const noexcept;

        vk::Result submit(const vk::SubmitInfo& submitInfo, vk::Fence fence = nullptr) const noexcept;

        vk::Result waitIdle() const noexcept;

        vk::Queue handle;
    };

private:
    VulkanDevice() = delete;

public:
    VulkanDevice(const VulkanPhysicalDevice& physicalDevice, const vk::DeviceCreateInfo& createInfo);

    VulkanDevice(const VulkanPhysicalDevice& physicalDevice);
    
    ~VulkanDevice();

    const vk::Device& getHandle() const noexcept { return *handle; }

    const VulkanPhysicalDevice& getPhysicalDevice() const noexcept { return physicalDevice; }

    const Queue& getGraphicsQueue() const noexcept;

    const VulkanCommandPool& getGraphicsCommandPool() const noexcept;

    bool hasAssociatedObject() const noexcept;

    juce::ReferenceCountedObject* getAssociatedObject(const char* name) const;

    void setAssociatedObject(const char* name, juce::ReferenceCountedObject* newObject);

    void clearAssociatedObjects();

    void waitIdle() const noexcept
    {
        jassert(handle);
        const auto result = handle->waitIdle();
        juce::ignoreUnused(result);
    }

private:
    const VulkanPhysicalDevice& physicalDevice;
    
    vk::UniqueDevice handle;

    juce::OwnedArray<const Queue> queues;

    const Queue* graphicsQueue = nullptr;
    std::unique_ptr<VulkanCommandPool> graphicsCommandPool;
    
    juce::StringArray associatedObjectNames;
    juce::ReferenceCountedArray<juce::ReferenceCountedObject> associatedObjects;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanDevice)
};

} // namespace parawave