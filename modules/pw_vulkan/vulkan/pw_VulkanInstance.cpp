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

namespace VulkanLoader
{

juce::StringArray getLayers() noexcept
{
    static juce::StringArray layers =
    {
    #if PW_VULKAN_ENABLE_VALIDATION_LAYERS
        "VK_LAYER_KHRONOS_validation",
    #endif
    };

    return layers;
}

juce::StringArray getExtensions() noexcept
{
    static juce::StringArray extensions =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,

    #if defined( VK_USE_PLATFORM_WIN32_KHR )
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
   
    #elif defined( VK_USE_PLATFORM_MACOS_MVK )
        VK_MVK_MACOS_SURFACE_EXTENSION_NAME,

    #elif defined( VK_USE_PLATFORM_IOS_MVK )
        VK_MVK_IOS_SURFACE_EXTENSION_NAME,

    #elif defined( VK_USE_PLATFORM_XLIB_KHR )
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME

    #elif defined(VK_USE_PLATFORM_ANDROID_KHR)
        VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,

    #endif

    #if PW_VULKAN_USE_DEBUG_UTILS
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    #endif
    };

    return extensions;
}

void initialiseDynamicLoader()
{
#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
    static vk::DynamicLoader dynamicLoader;
    
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
#endif
}

void initialiseInstanceFunctionPointer(const vk::Instance& instance)
{
    // Initialize function pointers for instance
#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
    jassert(instance);
    if (instance)
        VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
#endif
}

void getEnabledLayers(const juce::StringArray& layers, std::vector<const char*>& enabledLayers)
{
#if PW_VULKAN_ENABLE_VALIDATION_LAYERS
    vk::Result result;
    std::vector<vk::LayerProperties> layerProperties;

    std::tie(result, layerProperties) = vk::enumerateInstanceLayerProperties();
    
    PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't enumerate instance layer properties.");

    if (result != vk::Result::eSuccess)
        return;

#if (PW_VULKAN_PRINT_VALIDATION_LAYER_INFO == 1)
    PW_DBG_V("Available layers:");
    for (const auto& layer : layerProperties)
    {
        DBG(String("\t") + String(layer.layerName.data()));
    }
#endif

    enabledLayers.reserve(layers.size());
    
    for (const auto& layer : layers)
    {
        const auto available = std::find_if(layerProperties.begin(), layerProperties.end(), 
        [layer](const vk::LayerProperties& layerProperty) 
        {
            return layer == layerProperty.layerName;
        } 
        ) != layerProperties.end();

        if (available)
            enabledLayers.push_back(layer.toUTF8());
        else
            jassertfalse; // Requested layer not available in driver.
    }

#if (PW_VULKAN_PRINT_VALIDATION_LAYER_INFO == 1)
    PW_DBG_V("Enabled layers:");
    for (const auto& layer : enabledLayers) 
    {
        DBG(String("\t") + String(layer));
    }
#endif

#else
    juce::ignoreUnused(layers);
    juce::ignoreUnused(enabledLayers);
#endif
}

void getEnabledExtensions(const juce::StringArray& extensions, std::vector<const char*>& enabledExtensions)
{
    vk::Result result;
    std::vector<vk::ExtensionProperties> extensionProperties;

    std::tie(result, extensionProperties) = vk::enumerateInstanceExtensionProperties();
    
    PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't enumerate instance extension properties.");

    if (result != vk::Result::eSuccess)
        return;

#if (PW_VULKAN_PRINT_INSTANCE_EXTENSIONS_INFO == 1)
    PW_DBG_V("Available extensions:");
    for (const auto& extension : extensionProperties) 
    {
        DBG(String("\t") + String(extension.extensionName.data()));
    }
#endif

    enabledExtensions.reserve(extensions.size());
    
    for (const auto& extension : extensions)
    {
        const auto available = std::find_if(extensionProperties.begin(), extensionProperties.end(), 
        [extension](const vk::ExtensionProperties& extensionProperty) 
        {
            return extension == extensionProperty.extensionName;
        } 
        ) != extensionProperties.end();
        
        if (available)
            enabledExtensions.push_back(extension.toUTF8());
        else
            jassertfalse; // Requested extension not available in driver.
    }

#if (PW_VULKAN_PRINT_INSTANCE_EXTENSIONS_INFO == 1)
    PW_DBG_V("Enabled extensions:");
    for (const auto& layer : enabledExtensions) 
    {
        DBG(String("\t") + String(layer));
    }
#endif
}

} // namespace VulkanLoader

//==============================================================================

VulkanInstance::CreateInfo::CreateInfo(uint32_t apiVersion)
{
    using namespace VulkanLoader;
        
    initialiseDynamicLoader();
    
    getEnabledLayers(getLayers(), enabledLayers);
    getEnabledExtensions(getExtensions(), enabledExtensions);

    applicationInfo
        .setPApplicationName(nullptr)
        .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
        .setPEngineName(nullptr)
        .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
        .setApiVersion(apiVersion);

    setPApplicationInfo(&applicationInfo);
    setPEnabledLayerNames(enabledLayers);
    setPEnabledExtensionNames(enabledExtensions);

    // Addionally use the debugUtilsMessengerCallback in instance creation!
#if PW_VULKAN_USE_DEBUG_UTILS
    debugInfo = VulkanDebugUtilsMessenger::getDefaultCreateInfo();
    setPNext(&debugInfo);
#endif
}

//==============================================================================

VulkanInstance::VulkanInstance(const vk::InstanceCreateInfo& createInfo)
{
    vk::Result result;

    std::tie(result, handle) = vk::createInstanceUnique(createInfo).asTuple();

    PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't create instance.");

    if (result == vk::Result::eSuccess && handle)
    {
        version = createInfo.pApplicationInfo->apiVersion;

        DBG(getVersionString());
        PW_DBG_V("Created instance.");

        VulkanLoader::initialiseInstanceFunctionPointer(getHandle());

        #if PW_VULKAN_USE_DEBUG_UTILS
            debugUtilsMessenger = std::make_unique<VulkanDebugUtilsMessenger>(*this, VulkanDebugUtilsMessenger::getDefaultCreateInfo());
        #endif

        enumeratePhysicalDevices();
    } 
}

VulkanInstance::~VulkanInstance()
{
    PW_DBG_V("Destroyed instance.");
}

juce::String VulkanInstance::getVersionString() const noexcept
{
    if (version == 0)
        return "undefined";

    juce::String versionString;
    versionString << "VULKAN v" << juce::String(VK_VERSION_MAJOR(version)) 
                  << "." << juce::String(VK_VERSION_MINOR(version)) 
                  << "." << juce::String(VK_VERSION_PATCH(version));
    
    return versionString;
}

void VulkanInstance::enumeratePhysicalDevices()
{
    vk::Result result;
    std::vector<vk::PhysicalDevice> devices;

    jassert(handle);
    std::tie(result, devices) = handle->enumeratePhysicalDevices();

    PW_CHECK_VK_RESULT_SUCCESS(result, "Failed to enumerate physcial devices.");

    if (result != vk::Result::eSuccess)
        return;
        
    if (devices.empty())
    {
        PW_DBG_V("Failed to find GPUs with Vulkan support.");
        jassertfalse;
        return;
    }
        
    for (auto device : devices)
        physicalDevices.add(new VulkanPhysicalDevice(*this, device));
}
    
} // namespace parawave