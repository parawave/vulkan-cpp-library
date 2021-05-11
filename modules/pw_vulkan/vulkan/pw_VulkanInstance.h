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
    VulkanInstance

    There is no global state in Vulkan and all per-application state is stored 
    in an instance object. Creating an instance object initializes the Vulkan 
    library and allows the application to pass information about itself to the 
    implementation.
*/
class VulkanInstance final
{
public:
    static constexpr auto apiVersion1_0 = static_cast<uint32_t>(VK_API_VERSION_1_0);
    static constexpr auto apiVersion1_1 = static_cast<uint32_t>(VK_API_VERSION_1_1);
    static constexpr auto apiVersion1_2 = static_cast<uint32_t>(VK_API_VERSION_1_2);

public:
    struct CreateInfo : public vk::InstanceCreateInfo
    {
        CreateInfo(uint32_t apiVersion);

        vk::ApplicationInfo applicationInfo;

        std::vector<const char*> enabledLayers;
        std::vector<const char*> enabledExtensions;

        vk::DebugUtilsMessengerCreateInfoEXT debugInfo;
    };

public:
    VulkanInstance(const vk::InstanceCreateInfo& createInfo);

    explicit VulkanInstance(uint32_t apiVersion)
        : VulkanInstance(CreateInfo(apiVersion)) {}

    VulkanInstance() : VulkanInstance(apiVersion1_0) {}

    ~VulkanInstance();

    const vk::Instance& getHandle() const noexcept { return *handle; }

    uint32_t getVersion() const noexcept { return version; }

    juce::String getVersionString() const noexcept;

    const juce::OwnedArray<const VulkanPhysicalDevice>& getPhysicalDevices() const noexcept { return physicalDevices; }

private:
    void enumeratePhysicalDevices();

private:
    vk::UniqueInstance handle;

    juce::OwnedArray<const VulkanPhysicalDevice> physicalDevices;

    std::unique_ptr<VulkanDebugUtilsMessenger> debugUtilsMessenger;

    uint32_t version = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanInstance)
};
    
} // namespace parawave