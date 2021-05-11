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

/*******************************************************************************

 BEGIN_JUCE_MODULE_DECLARATION

  ID:               pw_vulkan_graphics
  vendor:           Parawave
  version:          0.5.0
  name:             Parawave - Vulkan Graphics
  description:      Vulkan Graphics Context for Component drawing.
  website:          https://parawave-audio.com/vulkan-cpp-library
  license:          ISC

  dependencies:     juce_core juce_data_structures juce_events juce_graphics juce_gui_basics pw_vulkan

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/

#pragma once

/*******************************************************************************
  JUCE SDK
*******************************************************************************/
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

/*******************************************************************************
  Parawave
*******************************************************************************/
#include <pw_vulkan/pw_vulkan.h>

/*******************************************************************************
  pw_vulkan_graphics
*******************************************************************************/
namespace parawave
{
    class VulkanContext;
    class VulkanImageType;
    class VulkanAppComponent;
} // namespace parawave

//==============================================================================

#include "contexts/pw_VulkanContext.h"

#include "utils/pw_VulkanUniform.h"
#include "utils/pw_VulkanIndexBuffer.h"
#include "utils/pw_VulkanImageType.h"
#include "utils/pw_VulkanTexture.h"
#include "utils/pw_VulkanAppComponent.h"