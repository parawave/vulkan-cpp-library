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
  
VulkanSurface::VulkanSurface(const VulkanPhysicalDevice& physicalDevice_)
    : physicalDevice(physicalDevice_) {}

VulkanSurface::~VulkanSurface() = default;

void VulkanSurface::updateCapabilities() noexcept
{
    jassert(physicalDevice.getHandle());

    vk::Result result;

    // Surface Capabilities
    std::tie(result, capabilities) = physicalDevice.getHandle().getSurfaceCapabilitiesKHR(*handle);
    PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't get surface capabilities.");

    // Surface Formats
    std::tie(result, surfaceFormats) = physicalDevice.getHandle().getSurfaceFormatsKHR(*handle);
    PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't get surface formats.");

    // Surface Present Modes
    std::tie(result, presentModes) = physicalDevice.getHandle().getSurfacePresentModesKHR(*handle);
    PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't get surface present modes");
}

bool VulkanSurface::isPresentModeSupported(vk::PresentModeKHR presentMode) const noexcept
{
    for (const auto& availableMode : presentModes) 
    {
        if (presentMode == availableMode)
            return true;
    }

    return false;
}

bool VulkanSurface::isFormatSupported(vk::Format format, vk::ColorSpaceKHR colorSpace) const noexcept
{
    for (const auto& availableFormat : surfaceFormats)
    {
        if (format == availableFormat.format && colorSpace == availableFormat.colorSpace)
            return true;
    }

    return false;
}

vk::SurfaceFormatKHR VulkanSurface::getDefaultFormat() const noexcept 
{ 
    return !surfaceFormats.empty() ? surfaceFormats.front() : vk::SurfaceFormatKHR(); 
}

vk::Extent2D VulkanSurface::getExtent() const noexcept
{
    const auto safeWidth = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, capabilities.currentExtent.width));
    const auto safeHeight = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, capabilities.currentExtent.height));

    return vk::Extent2D(safeWidth, safeHeight);
}

vk::SurfaceTransformFlagBitsKHR VulkanSurface::getTransform() const noexcept
{
    return capabilities.currentTransform;
}

juce::Range<uint32_t> VulkanSurface::getImageCount() const noexcept 
{ 
    return juce::Range<uint32_t>(capabilities.minImageCount, capabilities.maxImageCount); 
}

} // namespace parawave