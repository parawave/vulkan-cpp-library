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

namespace parawave
{
    
VulkanPhysicalDevice::VulkanPhysicalDevice(const VulkanInstance& instance_, const vk::PhysicalDevice& handle_)
    : instance(instance_), handle(handle_), memoryProperties(handle.getMemoryProperties())
{
    // Device Properties
    {
        auto properties = handle.getProperties();

        type = properties.deviceType;
        limits = properties.limits;
        
        name = properties.deviceName.operator std::string();
    }

    // Show available memory heaps
    #if(JUCE_DEBUG == 1)
    {
        const auto heapCount = memoryProperties.memoryHeapCount;
        for (auto i = 0U; i < heapCount; ++i)
        {
            auto& heap = memoryProperties.memoryHeaps[i];
            juce::ignoreUnused(heap);

            PW_DBG_V("Heap Size: " << juce::File::descriptionOfSizeInBytes(heap.size) << " Flags: " << vk::to_string(heap.flags));
        }
    }
    #endif

    // Queue Family Properties
    {
        auto properties = handle.getQueueFamilyProperties();
            
        auto index = 0U;

        for (const auto& p : properties)
        {
            QueueFamily family;

            family.index = index;
            family.count = p.queueCount;

            family.flags = p.queueFlags;
                
            queueFamilies.add(family);

            ++index;
        }
    }
}

bool VulkanPhysicalDevice::isSurfaceSupported(const vk::SurfaceKHR& surface) const noexcept
{
    for (const auto& queueFamily : queueFamilies)
    {
        vk::Result result;
        uint32_t supportedValue;

        std::tie(result, supportedValue) = handle.getSurfaceSupportKHR(queueFamily.index, surface);

        if (result == vk::Result::eSuccess)
        {
            if (supportedValue == VK_TRUE)
                return true;
        }
    }

    return false;
}

} // namespace parawave