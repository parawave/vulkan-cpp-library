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
    
VulkanAppComponent::VulkanAppComponent()
{
    setOpaque(true);
    
    for (auto& device : instance.getPhysicalDevices())
    {
        PW_DBG_V(device->getName() + " : " + device->getDeviceTypeName());
        juce::ignoreUnused(device);
    }

    context.setDefaultPhysicalDevice(instance);
    context.attachTo(*this);
}

VulkanAppComponent::~VulkanAppComponent()
{
    // Before your subclass's destructor has completed, you must call
    // shutdownVulkan() to release the Vulkan context. (Otherwise there's
    // a danger that it may invoke a Vulkan callback on your class while
    // it's in the process of being deleted.
    jassert(!context.isAttached());

    shutdownVulkan();
}

void VulkanAppComponent::shutdownVulkan()
{
    context.detach();
}

} // namespace parawave