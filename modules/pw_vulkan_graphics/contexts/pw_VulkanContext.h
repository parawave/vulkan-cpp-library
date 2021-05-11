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
    VulkanContext
*/
class VulkanContext
{
public:
    VulkanContext();
    ~VulkanContext();

    bool isFullscreen() const noexcept;

    /** Resize the target component to fit the whole screen.
        Behind the scenes this uses the juce::Desktop::setKioskModeComponent() method.
    
        Because changing the window state during rendering could cause a swapchain recreation,
        we manage the kiosk mode swiching in the context to make sure it's not destroying any
        device resources that are still in use. 
    */
    void setFullscreen(bool useFullscreen);

    void setFormat(vk::Format preferredFormat);

    void setColourSpace(vk::ColorSpaceKHR preferredColorSpace);

    void setPresentMode(vk::PresentModeKHR preferredPresentMode);

    void setPhysicalDevice(const VulkanPhysicalDevice& physicalDevice);

    void setDefaultPhysicalDevice(const VulkanInstance& instance);

    void resetPhysicalDevice();

    void attachTo(juce::Component&);

    void detach();

    bool isAttached() const noexcept;

    VulkanDevice* getDevice() const noexcept;

    double getRenderingScale() const noexcept;

    juce::Component* getTargetComponent() const noexcept;

    static VulkanContext* getContextAttachedTo(juce::Component& component) noexcept;

    void triggerRepaint();

private:
    class CachedImage;
    class Attachment;

    std::unique_ptr<VulkanDevice> device; 
    std::unique_ptr<Attachment> attachment;
   
    vk::Format format = vk::Format::eB8G8R8A8Unorm;
    vk::ColorSpaceKHR colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
    vk::PresentModeKHR presentMode = vk::PresentModeKHR::eMailbox;

    CachedImage* getCachedImage() const noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanContext)
};

} // namespace parawave