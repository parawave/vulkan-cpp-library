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

#include <JuceHeader.h>

#undef max
#undef min

/*******************************************************************************
  pw_vulkan
*******************************************************************************/
#include "pw_vulkan_graphics.h"

/** Define the type identifier of the VulkanImageType. Change this number in case of 
    conflicts with other juce::ImageType implementations that use the same id. */
#ifndef PW_VULKAN_IMAGE_TYPE_ID
  #define PW_VULKAN_IMAGE_TYPE_ID 4
#endif

//==============================================================================

namespace parawave
{
    class VulkanPixelData;
    class RenderLayer;

} // namespace parawave

//==============================================================================

// Compiled Binary SPV Shaders
#include "contexts/spv/pw_Basic_vert.cpp"
#include "contexts/spv/pw_Image_frag.cpp"
#include "contexts/spv/pw_Image_vert.cpp"
#include "contexts/spv/pw_LinearGradient_vert.cpp"
#include "contexts/spv/pw_LinearGradient1_frag.cpp"
#include "contexts/spv/pw_LinearGradient2_frag.cpp"
#include "contexts/spv/pw_Overlay_frag.cpp"
#include "contexts/spv/pw_Overlay_vert.cpp"
#include "contexts/spv/pw_RadialGradient_frag.cpp"
#include "contexts/spv/pw_RadialGradient_vert.cpp"
#include "contexts/spv/pw_SolidColour_frag.cpp"
#include "contexts/spv/pw_SolidColour_vert.cpp"
#include "contexts/spv/pw_TiledImage_frag.cpp"
#include "contexts/spv/pw_TiledImage_vert.cpp"

//==============================================================================

#include "contexts/caches/pw_CachedShaders.cpp"
#include "contexts/caches/pw_CachedMemory.cpp"
#include "contexts/caches/pw_CachedImages.cpp"
#include "contexts/caches/pw_CachedRenderPasses.cpp"

#include "contexts/shaders/pw_ProgramHelpers.cpp"
#include "contexts/shaders/pw_ImageProgram.cpp"
#include "contexts/shaders/pw_LinearGradientProgram.cpp"
#include "contexts/shaders/pw_OverlayProgram.cpp"
#include "contexts/shaders/pw_RadialGradientProgram.cpp"
#include "contexts/shaders/pw_SolidColourProgram.cpp"
#include "contexts/shaders/pw_TiledImageProgram.cpp"

#include "contexts/caches/pw_CachedPipelines.cpp"

#include "contexts/pw_DeviceState.cpp"
#include "contexts/pw_VulkanRenderer.cpp"

#include "contexts/renderer/pw_RenderHelpers.cpp"
#include "contexts/renderer/pw_RenderBase.cpp"
#include "contexts/renderer/pw_RenderFrame.cpp"
#include "contexts/renderer/pw_RenderLayer.cpp"

#include "contexts/pw_FrameState.cpp"
#include "contexts/pw_OverlayState.cpp"
#include "contexts/pw_RenderContext.cpp"

#include "contexts/pw_VulkanGraphicsContext.cpp"
#include "contexts/pw_VulkanContext.cpp"

#include "utils/pw_VulkanImageType.cpp"
#include "utils/pw_VulkanAppComponent.cpp"
