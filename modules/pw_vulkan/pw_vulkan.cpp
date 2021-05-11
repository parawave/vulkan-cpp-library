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

#define JUCE_GUI_BASICS_INCLUDE_SCOPED_THREAD_DPI_AWARENESS_SETTER 1

#include <JuceHeader.h>

/*******************************************************************************
  pw_vulkan
*******************************************************************************/
#include "pw_vulkan.h"

#ifndef PW_VULKAN_PRINT_VALIDATION_LAYER_INFO
  #define PW_VULKAN_PRINT_VALIDATION_LAYER_INFO 0
#endif

#ifndef PW_VULKAN_PRINT_INSTANCE_EXTENSIONS_INFO
  #define PW_VULKAN_PRINT_INSTANCE_EXTENSIONS_INFO 0
#endif

#ifndef PW_VULKAN_PRINT_DEVICE_EXTENSIONS_INFO
  #define PW_VULKAN_PRINT_DEVICE_EXTENSIONS_INFO 0
#endif

#if(JUCE_DEBUG)
  #define PW_VULKAN_ENABLE_VALIDATION_LAYERS 1
  #define PW_VULKAN_USE_DEBUG_UTILS 1
#endif

#ifndef PW_VULKAN_ENABLE_VALIDATION_LAYERS
  #define PW_VULKAN_ENABLE_VALIDATION_LAYERS 0
#endif

#ifndef PW_VULKAN_USE_DEBUG_UTILS
  #define PW_VULKAN_USE_DEBUG_UTILS 0
#endif

//==============================================================================

namespace parawave
{
    
} // namespace parawave

//==============================================================================
#include "vulkan/pw_Definitions.cpp"

#if JUCE_WINDOWS
 #include "native/pw_Vulkan_win32.h"

#elif JUCE_MAC || JUCE_IOS

 #if JUCE_MAC
  #include "native/pw_Vulkan_osx.h"
 #else
  #include "native/pw_Vulkan_ios.h"
 #endif

#elif JUCE_LINUX
 #include "native/pw_Vulkan_linux_X11.h"

#elif JUCE_ANDROID
 #include "native/pw_Vulkan_android.h"

#endif

//==============================================================================

#include "vulkan/pw_VulkanInstance.cpp"
#include "vulkan/pw_VulkanCommandBuffer.cpp"
#include "vulkan/pw_VulkanDebugUtilsMessenger.cpp"
#include "vulkan/pw_VulkanSurface.cpp"
#include "vulkan/pw_VulkanPhysicalDevice.cpp"
#include "vulkan/pw_VulkanDevice.cpp"
#include "vulkan/pw_VulkanSwapchain.cpp"
