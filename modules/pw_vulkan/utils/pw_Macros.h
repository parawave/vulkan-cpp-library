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

#if (JUCE_DEBUG)

  //==============================================================================
  /** Platform-independent assertion macro.

      vk::Result result = ...
      PW_CHECK_VK_RESULT_SUCCESS(result, "Error because ...");

      Is a short way of writing:

      if(result != vk::Result::eSuccess)
      {
          DBG("[Vulkan] Error because ...");
          jassertfalse;
      }
  */
  #define PW_CHECK_VK_RESULT_SUCCESS(result, textToWrite)     JUCE_BLOCK_WITH_FORCED_SEMICOLON (if (result != vk::Result::eSuccess) { DBG(juce::String("[Vulkan] ") << vk::to_string(result) << " : " << textToWrite); jassertfalse; })

  #define PW_CHECK_VK_RESULT(expression, result, textToWrite) JUCE_BLOCK_WITH_FORCED_SEMICOLON (if (! (expression)) { DBG(juce::String("[Vulkan] ") << vk::to_string(result) << " : " << textToWrite); jassertfalse; })

#else
  //==============================================================================
  // If debugging is disabled, these dummy debug and assertion macros are used..
  
  #define PW_CHECK_VK_RESULT_SUCCESS(result, textToWrite)       JUCE_BLOCK_WITH_FORCED_SEMICOLON (juce::ignoreUnused(result);)

  #define PW_CHECK_VK_RESULT(expression, result, textToWrite)   JUCE_BLOCK_WITH_FORCED_SEMICOLON (juce::ignoreUnused(result);)

#endif

//==============================================================================
#if (PW_SHOW_VERBOSE_DEBUG_MESSAGES == 1)
    #define PW_DBG_V(textToWrite)  JUCE_BLOCK_WITH_FORCED_SEMICOLON ( DBG(juce::String("[Vulkan] ") << textToWrite); )
#else
    #define PW_DBG_V(textToWrite)
#endif