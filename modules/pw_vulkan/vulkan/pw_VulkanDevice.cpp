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

namespace VulkanDeviceHelpers
{

juce::StringArray getRequiredExtensions() noexcept
{
    static juce::StringArray extensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    return extensions;
}

void getEnabledExtensions(const vk::PhysicalDevice& physicalDevice, const juce::StringArray& extensions, std::vector<const char*>& enabledExtensions)
{
    vk::Result result;
    std::vector<vk::ExtensionProperties> extensionProperties;

    std::tie(result, extensionProperties) = physicalDevice.enumerateDeviceExtensionProperties();

    if (result != vk::Result::eSuccess)
        return;

#if (PW_VULKAN_PRINT_DEVICE_EXTENSIONS_INFO == 1)
    PW_DBG_V("Available device extensions:");
    for (const auto& extension : extensionProperties)
    {
        DBG(String("\t") + String(extension.extensionName.data()));
    }
#endif

    enabledExtensions.reserve(extensions.size());

    for (const auto& extension : extensions)
    {
        const auto available = std::find_if
        (
            extensionProperties.begin(), extensionProperties.end(),
            [extension](const vk::ExtensionProperties& extensionProperty)
            {
                return extension == extensionProperty.extensionName;
            }
        ) != extensionProperties.end();

        if (available)
            enabledExtensions.push_back(extension.toUTF8());
        else
            jassertfalse; // Requested device extension not available in driver.
    }

#if (PW_VULKAN_PRINT_DEVICE_EXTENSIONS_INFO == 1)
    PW_DBG_V("Enabled device extensions:");
    for (const auto& layer : enabledExtensions)
    {
        DBG(String("\t") + String(layer));
    }
#endif
}

struct DeviceCreateInfo : public vk::DeviceCreateInfo
{
    DeviceCreateInfo(const VulkanPhysicalDevice& physicalDevice)
    {
        getEnabledExtensions(physicalDevice.getHandle(), getRequiredExtensions(), enabledExtensions);

        for (const auto& queueFamily : physicalDevice.getQueueFamilies())
        {
            queueCreateInfos.push_back
            (
                vk::DeviceQueueCreateInfo()
                .setQueueFamilyIndex(queueFamily.index)
                .setQueueCount(1)
                .setQueuePriorities(queuePriority)
            );
        }

        setQueueCreateInfos(queueCreateInfos);
        setPEnabledExtensionNames(enabledExtensions);
    }

    float queuePriority = 1.0f;
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    
    std::vector<const char*> enabledExtensions;
};

} // namespace VulkanDeviceHelpers

//==============================================================================
VulkanDevice::VulkanDevice(const VulkanPhysicalDevice& physicalDevice_, const vk::DeviceCreateInfo& createInfo)
    : physicalDevice(physicalDevice_)
{
    vk::Result result;
    
    jassert(physicalDevice.getHandle());
    std::tie(result, handle) = physicalDevice.getHandle().createDeviceUnique(createInfo).asTuple();

    PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't create device.");

    if (result == vk::Result::eSuccess && handle)
    {
        PW_DBG_V("Created device.");

        // Find the first graphics queue family and set it as main
        for (const auto& queueFamily : physicalDevice.getQueueFamilies())
        {
            if (queueFamily.isGraphicsQueue())
            {
                auto queue = std::make_unique<Queue>();
                queue->handle = handle->getQueue(queueFamily.index, 0);
            
                graphicsQueue = queues.add(queue.release());
                graphicsCommandPool.reset(new VulkanCommandPool(*this, queueFamily.index));
            
                break;
            }
        }

        jassert(graphicsQueue != nullptr);
    }
}

VulkanDevice::VulkanDevice(const VulkanPhysicalDevice& physicalDevice_) 
    : VulkanDevice(physicalDevice_, VulkanDeviceHelpers::DeviceCreateInfo(physicalDevice_)) {}

VulkanDevice::~VulkanDevice()
{
    waitIdle();

    PW_DBG_V("Destroyed device.");
}

const VulkanCommandPool& VulkanDevice::getGraphicsCommandPool() const noexcept
{
    return *graphicsCommandPool;
}

const VulkanDevice::Queue& VulkanDevice::getGraphicsQueue() const noexcept
{
    jassert(graphicsQueue != nullptr);
    return *graphicsQueue;
}

bool VulkanDevice::hasAssociatedObject() const noexcept
{
    return !associatedObjectNames.isEmpty();
}

juce::ReferenceCountedObject* VulkanDevice::getAssociatedObject (const char* name) const
{
    jassert (name != nullptr);

    auto index = associatedObjectNames.indexOf (name);
    return index >= 0 ? associatedObjects.getUnchecked (index).get() : nullptr;
}

void VulkanDevice::setAssociatedObject (const char* name, juce::ReferenceCountedObject* newObject)
{
    jassert (name != nullptr);

    const int index = associatedObjectNames.indexOf (name);

    if (index >= 0)
    {
        if (newObject != nullptr)
        {
            associatedObjects.set (index, newObject);
        }
        else
        {
            associatedObjectNames.remove (index);
            associatedObjects.remove (index);
        }
    }
    else if (newObject != nullptr)
    {
        associatedObjectNames.add (name);
        associatedObjects.add (newObject);
    }
}

void VulkanDevice::clearAssociatedObjects()
{
    waitIdle();

    associatedObjectNames.clear();
    associatedObjects.clear();
}

//==============================================================================

vk::Result VulkanDevice::Queue::submit(const VulkanCommandBuffer& commandBuffer, vk::Fence fence) const noexcept
{
    std::array<vk::CommandBuffer, 1> buffers = { commandBuffer.getHandle() };

    const auto submitInfo = vk::SubmitInfo()
        .setCommandBuffers(buffers);

    return submit(submitInfo, fence);
}

vk::Result VulkanDevice::Queue::submit(const vk::SubmitInfo& submitInfo, vk::Fence fence) const noexcept
{
    jassert(handle);
    const auto result = handle.submit(submitInfo, fence);

    PW_CHECK_VK_RESULT_SUCCESS(result, "Failed to submit to queue.");

    return result;
}

vk::Result VulkanDevice::Queue::waitIdle() const noexcept
{
    jassert(handle);
    const auto result = handle.waitIdle();

    PW_CHECK_VK_RESULT_SUCCESS(result, "Failed to wait for queue.");
    
    return result;
}

} // namespace parawave