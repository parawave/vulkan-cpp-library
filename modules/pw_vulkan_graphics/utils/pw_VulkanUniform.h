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
    VulkanUniform

    Useful shader uniform or push constants structures.
*/
class VulkanUniform final
{
public:
    //==============================================================================
    struct ScreenBounds
    {
        float values[4];

        void set(const juce::Rectangle<float>& bounds) noexcept
        {
            values[0] = bounds.getX();
            values[1] = bounds.getY();
            values[2] = 0.5f * bounds.getWidth();
            values[3] = 0.5f * bounds.getHeight();
        }
    };

    //==============================================================================
    struct Matrix
    {
        float values[6];

        void set(juce::AffineTransform t) noexcept
        {
            values[0] = t.mat00; values[1] = t.mat01; values[2] = t.mat02;
            values[3] = t.mat10; values[4] = t.mat11; values[5] = t.mat12;
        }

        void setIdentity() noexcept
        {
            set(juce::AffineTransform());
        }
    };

    //==============================================================================
    struct Colour
    {
        float values[4];

        void set(const juce::Colour colour) noexcept
        {
            values[0] = colour.getFloatRed();
            values[1] = colour.getFloatGreen();
            values[2] = colour.getFloatBlue();
            values[3] = colour.getFloatAlpha();
        }

        void set(juce::PixelARGB colour) noexcept
        {
            set(juce::Colour(colour));
        }
    };
};

} // namespace parawave
