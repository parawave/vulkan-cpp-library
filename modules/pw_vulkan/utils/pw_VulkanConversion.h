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
    VulkanConversion

    Helper functions to convert between juce and vk objects like 
    point, rectangle, extent, viewport, ...
*/
class VulkanConversion final
{
public:
    forcedinline static vk::Offset2D toOffset2D(const juce::Point<int>& offset) noexcept
    {
        return { offset.x, offset.y };
    }

    forcedinline static vk::Offset2D toOffset2D(const juce::Rectangle<int>& bounds) noexcept
    {
        return toOffset2D(bounds.getPosition());
    }

    forcedinline static vk::Extent2D toExtent2D(const juce::Rectangle<int>& bounds) noexcept
    {
        return { static_cast<uint32_t>(bounds.getWidth()), static_cast<uint32_t>(bounds.getHeight()) };
    }

    forcedinline static vk::Extent3D toExtent3D(const juce::Rectangle<int>& bounds, uint32_t depth) noexcept
    {
        return { static_cast<uint32_t>(bounds.getWidth()), static_cast<uint32_t>(bounds.getHeight()), depth };
    }

    forcedinline static vk::Rect2D toRect2D(const juce::Rectangle<int>& bounds) noexcept
    {
        return { toOffset2D(bounds), toExtent2D(bounds) };
    }

    forcedinline static vk::Viewport toViewport(const vk::Rect2D rect) noexcept
    {
        return { static_cast<float>(rect.offset.x), 
                 static_cast<float>(rect.offset.y), 
                 static_cast<float>(rect.extent.width), 
                 static_cast<float>(rect.extent.height), 
                 0.0f, 1.0f };
    }

    forcedinline static juce::Rectangle<int> toRectangle(const vk::Extent2D extent)
    {
        return { static_cast<int>(extent.width), static_cast<int>(extent.height) };
    }

    forcedinline static juce::Rectangle<int> toRectangle(const vk::Extent3D extent)
    {
        return { static_cast<int>(extent.width), static_cast<int>(extent.height) };
    }

    forcedinline static juce::Rectangle<float> toRectangle(const vk::Viewport viewport) noexcept
    {
        return { viewport.x, viewport.y, viewport.width, viewport.height };
    }

    forcedinline static juce::Rectangle<int> toRectangle(const vk::Rect2D rect) noexcept
    {
        return { rect.offset.x, rect.offset.y, static_cast<int>(rect.extent.width), static_cast<int>(rect.extent.height) };
    }

    forcedinline static vk::ClearColorValue toClearColorValue(const juce::Colour colour) noexcept
    {
        return { std::array<float, 4>{ colour.getFloatRed(), colour.getFloatGreen(), colour.getFloatBlue(), colour.getFloatAlpha() } };
    }

    forcedinline static uint32_t toPackedColour(juce::PixelARGB colour) noexcept
	{
	#if JUCE_BIG_ENDIAN
		auto rgba = static_cast<uint32_t>((colour.getRed() << 24) | (colour.getGreen() << 16) | (colour.getBlue() << 8) |  colour.getAlpha());
	#else
		// vk::Format::eA8B8G8R8UnormPack32
        auto rgba = static_cast<uint32_t>((colour.getAlpha() << 24) | (colour.getBlue() << 16) | (colour.getGreen() << 8) |  colour.getRed());
	#endif
		return rgba;
	}
};

} // namespace parawave