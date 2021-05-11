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
    VulkanSwapchain

    A swapchain object (a.k.a. swapchain) provides the ability to present 
    rendering results to a surface. 
*/
class VulkanSwapchain final
{
private:
    VulkanSwapchain() = delete;

public:
    struct CreateInfo : public vk::SwapchainCreateInfoKHR
    {
        CreateInfo(const VulkanDevice& device, const VulkanSurface& surface, 
            vk::Format preferredFormat, vk::ColorSpaceKHR preferredColorSpace, vk::PresentModeKHR preferredPresentModeconst, 
            const VulkanSwapchain* oldSwapChain);

        bool isValid() const noexcept;
    };

public:
    VulkanSwapchain(const VulkanDevice& device, const VulkanSurface& surface, const vk::SwapchainCreateInfoKHR& createInfo);

    ~VulkanSwapchain();

    const VulkanDevice& getDevice() const noexcept { return device; }

    const vk::SwapchainKHR& getHandle() const noexcept { return *handle; }

    const vk::Format& getImageFormat() const noexcept { return surfaceFormat.format; }

    const vk::ColorSpaceKHR& getColorSpace() const noexcept { return surfaceFormat.colorSpace; }

    const vk::PresentModeKHR& getPresentMode() const noexcept { return presentMode; }

    const vk::Extent2D& getExtent() const noexcept { return extent; }

    uint32_t getWidth() const noexcept { return extent.width; }

    uint32_t getHeight() const noexcept { return extent.height; }

    vk::Viewport getViewport() const noexcept;

    size_t getNumImages() const noexcept;

    vk::Image getImage(size_t index) const noexcept;

    vk::Result acquireNextImage(uint32_t& imageIndex, const VulkanSemaphore& signalSemaphore, juce::RelativeTime timeout = juce::RelativeTime::seconds(1.0)) const noexcept;

    vk::Result presentImage(uint32_t imageIndex, const VulkanSemaphore& waitSemaphore) const noexcept;

private:
    void getSwapchainImages();

private:
    const VulkanDevice& device;

    vk::UniqueSwapchainKHR handle;

    vk::SurfaceFormatKHR surfaceFormat;
    vk::Extent2D extent;
    vk::SurfaceTransformFlagBitsKHR surfaceTransform;

    vk::PresentModeKHR presentMode;

    std::vector<vk::Image> images;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanSwapchain)
};
    
} // namespace parawave