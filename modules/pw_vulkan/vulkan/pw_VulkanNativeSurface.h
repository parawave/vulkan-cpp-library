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
    VulkanNativeSurface 

    This interface can be used to implement the platform specific creation and 
    repaint management of VulkanSurface 
    
    A valid extent and scale must be returned and position update and invalidation
    must trigger a repaint of the native surface.
*/
class VulkanNativeSurface
{
public:
    class Target
    {
    public:
        Target() = default;
        virtual ~Target() = default;

        /** The component the native surface is attached to. */
        virtual juce::Component& getSurfaceComponent() = 0;

        /** This will immediately trigger the frame render code in the surface target. */
        virtual void renderFrame() = 0;

        /** The rate of the native surface redraw in milliseconds. */
        virtual uint32_t getRefreshRate() const = 0;
    };

private:
    VulkanNativeSurface() = delete;

public:
    virtual ~VulkanNativeSurface() = default;

    /** A VulkanNativeSurface has a Target that requires a juce::Component, so it must be able to
        create a VulkanSurface from a VulkanPhysicalDevice.
    */
    virtual std::unique_ptr<VulkanSurface> createSurface(const VulkanPhysicalDevice& physicalDevice) const noexcept = 0;

    virtual vk::Extent2D getSurfaceExtent() const noexcept = 0;

    virtual double getSurfaceScale() const noexcept = 0;

    virtual void updateSurfacePosition(juce::Rectangle<int> bounds) const noexcept = 0;

    virtual void invalidateSurface() noexcept = 0;

    //==============================================================================
    //** If a native surface target is provided, a NativeSurface is created. */
    static std::unique_ptr<VulkanNativeSurface> create(VulkanNativeSurface::Target& surfaceTarget);

protected:
    VulkanNativeSurface(Target& surfaceTarget) : target(surfaceTarget) { }
    
    Target& target;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanNativeSurface)
};

} // namespace parawave