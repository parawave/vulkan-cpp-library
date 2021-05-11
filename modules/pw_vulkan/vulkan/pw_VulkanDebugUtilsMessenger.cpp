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

namespace parawave
{
    
VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* /*pUserData*/)
{
    juce::StringArray lines;

    lines.add(vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) + " : " + vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes)));
    lines.add("messageIdName = <" + juce::String(pCallbackData->pMessageIdName) + ">");
    lines.add("messageIdNumber = <" + juce::String(pCallbackData->messageIdNumber) + ">");
    lines.add("message = <" + juce::String(pCallbackData->pMessage) + ">");
 
    if ( 0 < pCallbackData->queueLabelCount)
    {
        lines.add("\t" + juce::String("Queue Labels:"));

        for (uint8_t i = 0; i < pCallbackData->queueLabelCount; ++i)
            lines.add("\t\t" + juce::String("labelName = <") + juce::String(pCallbackData->pQueueLabels[i].pLabelName) + ">");
    }

    if ( 0 < pCallbackData->cmdBufLabelCount )
    {
        lines.add("\t" + juce::String("CommandBuffer Labels:"));

        for (uint8_t i = 0; i < pCallbackData->cmdBufLabelCount; ++i)
            lines.add("\t\t" + juce::String("labelName = <") + juce::String(pCallbackData->pCmdBufLabels[i].pLabelName) + ">");
    }

    if ( 0 < pCallbackData->objectCount )
    {
        lines.add("\t" + juce::String("Objects:"));

        for (uint8_t i = 0; i < pCallbackData->objectCount; ++i)
        {
            lines.add("\t\t" + juce::String("Object ") + juce::String(i));
            lines.add("\t\t\t" + juce::String("objectType   = ") + juce::String(vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType))));
            lines.add("\t\t\t" + juce::String("objectHandle = ") + juce::String(pCallbackData->pObjects[i].objectHandle));

            if (pCallbackData->pObjects[i].pObjectName)
                lines.add("\t\t\t" + juce::String("objectName   = <") + juce::String(pCallbackData->pObjects[i].pObjectName) + ">");
        }
    }

    lines.add({});

    DBG(lines.joinIntoString("\r\n"));

    // Using an assertion here could help tracking the source of validation erros.
    //jassertfalse;
    
    return VK_TRUE;
}

vk::DebugUtilsMessengerCreateInfoEXT VulkanDebugUtilsMessenger::getDefaultCreateInfo() noexcept
{
    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags
    (
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | 
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | 
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
    );

     // TODO: Add optional verbose debug messages via macro: vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags
    ( 
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | 
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
    );

    const auto createInfo = vk::DebugUtilsMessengerCreateInfoEXT()
        .setMessageType(messageTypeFlags)
        .setMessageSeverity(severityFlags)
        .setPfnUserCallback(&debugUtilsMessengerCallback);

    return createInfo;
}

} // namespace parawave