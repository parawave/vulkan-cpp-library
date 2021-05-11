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

/*******************************************************************************
  VULKAN SDK
*******************************************************************************/

/** Use the default dynamic dispatch loader */
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

/** Use JUCE assertion instead of default <cassert> */
#define VULKAN_HPP_ASSERT jassert

/** Turn off assertions on result values */
#define VULKAN_HPP_ASSERT_ON_RESULT

/** Use typesafe conversions */
#define VULKAN_HPP_TYPESAFE_CONVERSION

/** Turn of exceptions */
#define VULKAN_HPP_NO_EXCEPTIONS

/** Use forced inline attribute to mark all Vulkan functions as inline */
#define VULKAN_HPP_INLINE forcedinline

/** Platform specific Khronos Extensions */
#if JUCE_WINDOWS
  #define VK_USE_PLATFORM_WIN32_KHR

  /** vulkan.hpp includes vulkan.h which unfortunately includes <windows.h>
      To avoid ambiguous symbols, especially juce::Rectangle, define NOGDI to avoid including these definitiosn.
  */
  #define NOGDI
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX

#elif JUCE_MAC || JUCE_IOS

 #if JUCE_MAC
  #define VK_USE_PLATFORM_MACOS_MVK
 #else
  #define VK_USE_PLATFORM_IOS_MVK
 #endif

 // #define VK_USE_PLATFORM_METAL_EXT ? Is this necessary ?

#elif JUCE_LINUX
  #define VK_USE_PLATFORM_XLIB_KHR

#elif JUCE_ANDROID
  #define VK_USE_PLATFORM_ANDROID_KHR

#endif

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunused")

/** Add Header Search Paths for Vulkan SDK to your Projucer project: 
    e.g C:\VulkanSDK\1.2.162.1\Include */
#include <vulkan/vulkan.hpp>

/** Some examples include the SDK Platform. This doesn't seem to be necessary? */
//#include <vulkan/vk_sdk_platform.h>

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

/** Undefine the usual min max macro madness after include of <windows.h>. */
#if JUCE_WINDOWS
  #undef ASSERT
  #undef WARNING
  #undef PRINTSYSERROR
  #undef DEBUGSTR
  #undef DBPRT0
  #undef DBPRT1
  #undef DBPRT2
  #undef DBPRT3
  #undef DBPRT4
  #undef DBPRT5
  #undef min
  #undef max
  #undef MIN
  #undef MAX
  #undef calloc
  #undef free
  #undef malloc
  #undef realloc
  #undef rad1
  #undef small
  #undef NEW
  #undef NEWVEC
  #undef VERIFY
  #undef VERIFY_IS
  #undef VERIFY_NOT
  #undef META_CREATE_FUNC
  #undef CLASS_CREATE_FUNC
  #undef SINGLE_CREATE_FUNC
  #undef _META_CLASS
  #undef _META_CLASS_IFACE
  #undef _META_CLASS_SINGLE
  #undef META_CLASS
  #undef META_CLASS_IFACE
  #undef META_CLASS_SINGLE
  #undef SINGLETON
  #undef OBJ_METHODS
  #undef QUERY_INTERFACE
  #undef LICENCE_UID
  #undef BEGIN_FACTORY
  #undef DEF_CLASS
  #undef DEF_CLASS1
  #undef DEF_CLASS2
  #undef DEF_CLASS_W
  #undef END_FACTORY
#endif
