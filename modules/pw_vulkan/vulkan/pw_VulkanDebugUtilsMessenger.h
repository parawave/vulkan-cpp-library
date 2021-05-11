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
    VulkanDebugUtilsMessenger

    Represents a debug utility messenger. 
    
    The debug messenger will provide detailed feedback on the application’s use 
    of Vulkan when events of interest occur. When an event of interest does 
    occur, the debug messenger will submit a debug message to the debug callback 
    that was provided during its creation. Additionally, the debug messenger is 
    responsible with filtering out debug messages that the callback is not 
    interested in and will only provide desired debug messages.
*/
class VulkanDebugUtilsMessenger final
{
private:
    VulkanDebugUtilsMessenger() = delete;

public:
    VulkanDebugUtilsMessenger(const VulkanInstance& instance, const vk::DebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        vk::Result result;

        jassert(instance.getHandle());
        std::tie(result, handle) = instance.getHandle().createDebugUtilsMessengerEXTUnique(createInfo).asTuple();

        PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't create debug utils messenger");
    }

    const vk::DebugUtilsMessengerEXT& getHandle() const noexcept { return *handle; }

    static vk::DebugUtilsMessengerCreateInfoEXT getDefaultCreateInfo() noexcept;

private:
    vk::UniqueDebugUtilsMessengerEXT handle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanDebugUtilsMessenger)
};

} // namespace parawave