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

  ID:               pw_vulkan
  vendor:           Parawave
  version:          0.5.0
  name:             Parawave - Vulkan
  description:      Vulkan Classes.
  website:          https://parawave-audio.com/vulkan-cpp-library
  license:          ISC

  dependencies:     juce_core juce_data_structures juce_events juce_graphics juce_gui_basics

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/

#pragma once

/*******************************************************************************
  Requirments
********************************************************************************

  This module requires the Vulkan SDK. 
  https://vulkan.lunarg.com/sdk/home

  You may also want to install the additional SDK Components. 
  https://files.lunarg.com/

  e.g.

    Vulkan SDK = VulkanSDK-1.2.162.1-Installer.exe

    Additional SDK Components = VulkanSDK-1.2.162.1-DebugLibs.zip

  Don't forget to add the SDK paths to your Projucer project.

    Header Search Paths
     C:\VulkanSDK\1.2.162.1\Include

    Extra Library Search Path
     C:\VulkanSDK\1.2.162.1\Lib

********************************************************************************
  Screen Tear Issues?
********************************************************************************

  If you are experiencing screen tearing even though the swap chain uses a V-Sync 
  present mode, it could be due to the global settings of your graphics card.
    
  If an eco mode or power saving plan is enabled this can result in stuttering or 
  tearing, for optimal visual fidelity and testing you should disable them!

*******************************************************************************/

//==============================================================================
/** Config: PW_SHOW_VERBOSE_DEBUG_MESSAGES

    If activated, more debug messages are print to see the initialisation order
    of Vulkan objects.
*/
#ifndef PW_SHOW_VERBOSE_DEBUG_MESSAGES
  #define PW_SHOW_VERBOSE_DEBUG_MESSAGES 0
#endif

/*******************************************************************************
  STL
*******************************************************************************/
#include <array>
#include <algorithm>

/*******************************************************************************
  JUCE SDK
*******************************************************************************/
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

/*******************************************************************************
  pw_vulkan
*******************************************************************************/
namespace parawave
{
    class VulkanBuffer;
    class VulkanBufferView;
    class VulkanCommandBuffer;
    class VulkanCommandPool;
    class VulkanContext;
    class VulkanDebugUtilsMessenger;
    class VulkanDescriptorPool;
    class VulkanDescriptorSet;
    class VulkanDescriptorSetLayout;
    class VulkanDevice;
    class VulkanDeviceMemory;
    class VulkanFence;
    class VulkanFramebuffer;
    class VulkanImage;
    class VulkanImageView;
    class VulkanInstance;
    class VulkanNativeSurface;
    class VulkanPipeline;
    class VulkanPipelineLayout;
    class VulkanPhysicalDevice;
    class VulkanRenderPass;
    class VulkanSampler;
    class VulkanSemaphore;
    class VulkanShaderModule;
    class VulkanSurface;
    class VulkanSurfaceKHR;
    class VulkanSwapchain;
    
} // namespace parawave

//==============================================================================

#include "vulkan/pw_Definitions.h"

#include "utils/pw_Macros.h"
#include "utils/pw_VulkanConversion.h"

#include "vulkan/pw_VulkanInstance.h"
#include "vulkan/pw_VulkanDebugUtilsMessenger.h"
#include "vulkan/pw_VulkanPhysicalDevice.h"
#include "vulkan/pw_VulkanDevice.h"
#include "vulkan/pw_VulkanSwapchain.h"
#include "vulkan/pw_VulkanNativeSurface.h"
#include "vulkan/pw_VulkanSurface.h"
#include "vulkan/pw_VulkanCommandPool.h"
#include "vulkan/pw_VulkanDeviceMemory.h"
#include "vulkan/pw_VulkanFence.h"
#include "vulkan/pw_VulkanSemaphore.h"
#include "vulkan/pw_VulkanDescriptorPool.h"
#include "vulkan/pw_VulkanDescriptorSetLayout.h"
#include "vulkan/pw_VulkanDescriptorSet.h"
#include "vulkan/pw_VulkanSampler.h"
#include "vulkan/pw_VulkanRenderPass.h"

#include "vulkan/pw_VulkanShaderModule.h"
#include "vulkan/pw_VulkanPipelineLayout.h"
#include "vulkan/pw_VulkanPipeline.h"
#include "vulkan/pw_VulkanCommandBuffer.h"
#include "vulkan/pw_VulkanBuffer.h"
#include "vulkan/pw_VulkanBufferView.h"
#include "vulkan/pw_VulkanImage.h"
#include "vulkan/pw_VulkanImageView.h"
#include "vulkan/pw_VulkanFramebuffer.h"

#include "memory/pw_VulkanMemoryRange.h"
#include "memory/pw_VulkanMemory.h"
#include "memory/pw_VulkanMemoryPool.h"
#include "memory/pw_VulkanMemoryBuffer.h"
#include "memory/pw_VulkanMemoryImage.h"

#include "descriptor/pw_VulkanDescriptorSetPool.h"
#include "descriptor/pw_VulkanDescriptor.h"

#include "utils/pw_VulkanCommandSequence.h"
#include "utils/pw_VulkanBufferTransfer.h"
#include "utils/pw_VulkanImageTransfer.h"
#include "utils/pw_VulkanComputePipeline.h"
#include "utils/pw_VulkanGraphicsPipeline.h"