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
    VulkanSurface

    Native platform surface or window objects are abstracted by surface objects, 
    which are represented by vk::SurfaceKHR handles:
*/
class VulkanSurface
{
private:
    VulkanSurface() = delete;

public:
    VulkanSurface(const VulkanPhysicalDevice& physicalDevice);
    virtual ~VulkanSurface();

    const vk::SurfaceKHR& getHandle() const noexcept { return *handle; }

    bool isPresentModeSupported(vk::PresentModeKHR presentMode) const noexcept;

    bool isFormatSupported(vk::Format format, vk::ColorSpaceKHR colorSpace) const noexcept;

    vk::SurfaceFormatKHR getDefaultFormat() const noexcept ;

    vk::Extent2D getExtent() const noexcept;

    vk::SurfaceTransformFlagBitsKHR getTransform() const noexcept;

    juce::Range<uint32_t> getImageCount() const noexcept;

    void updateCapabilities() noexcept;

private:
    const VulkanPhysicalDevice& physicalDevice;

protected:
    vk::UniqueSurfaceKHR handle;

private:
    vk::SurfaceCapabilitiesKHR capabilities;
    
    std::vector<vk::SurfaceFormatKHR> surfaceFormats;
    std::vector<vk::PresentModeKHR> presentModes;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanSurface)
};
    
} // namespace parawave