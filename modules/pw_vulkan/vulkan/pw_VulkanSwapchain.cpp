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

VulkanSwapchain::CreateInfo::CreateInfo(const VulkanDevice& /*device*/, const VulkanSurface& surface, 
    vk::Format preferredFormat, vk::ColorSpaceKHR preferredColorSpace, vk::PresentModeKHR preferredPresentMode, const VulkanSwapchain* oldSwapChain)
{
    // Image Count
    const auto imageCounts = surface.getImageCount();

    auto imageCount = imageCounts.getStart() + 1U;

    if (imageCounts.getEnd() > 0 && imageCount > imageCounts.getEnd()) 
        imageCount = imageCounts.getEnd();

    // TODO: Limited to 1 or 2 swapchain frames, make this a configuration parameter ?
    imageCount = juce::jlimit(1U, 2U, imageCount);
        
    // Default Values
    auto swapchainFormat = vk::SurfaceFormatKHR();    
    auto swapchainExtent = vk::Extent2D();
    auto swapchainTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;

    auto sharingMode = vk::SharingMode::eExclusive;
    auto swapchainPresentMode = vk::PresentModeKHR::eFifo;
        
    // Format & Color Space
    if (surface.isFormatSupported(preferredFormat, preferredColorSpace))
    {
        swapchainFormat.format = preferredFormat;
        swapchainFormat.colorSpace = preferredColorSpace;
    }

    if (swapchainFormat.format == vk::Format::eUndefined)
        swapchainFormat = surface.getDefaultFormat();

    swapchainExtent = surface.getExtent();

    swapchainTransform = surface.getTransform();

    /** If the graphics and present queues are from different queue families, we either have to explicitly transfer
        ownership of images between the queues, or we have to create the swapchain with imageSharingMode as VK_SHARING_MODE_CONCURRENT */
    std::vector<uint32_t> queueFamilyIndices;

    //const auto familyIndex = device.getPhysicalDevice().getQueueFamilies().getFirst().index;
    //queueFamilyIndices.push_back(familyIndex);

    {
        uint32_t graphicsFamily = 0;
        uint32_t presentFamily = 0;

        if (graphicsFamily != presentFamily) 
            sharingMode = vk::SharingMode::eConcurrent;
    }

    if (sharingMode != vk::SharingMode::eExclusive)
    {
        // TODO: Add other family index
        // queueFamilyIndices.
        
        PW_DBG_V("Concurrent sharing mode not implemented!");
        jassertfalse;
    }
       
    // Present Mode
    if (surface.isPresentModeSupported(preferredPresentMode))
        swapchainPresentMode = preferredPresentMode;
  
    setSurface(surface.getHandle());
    setMinImageCount(imageCount);
    setImageFormat(swapchainFormat.format);
    setImageColorSpace(swapchainFormat.colorSpace);
    setImageExtent(swapchainExtent);
    setImageArrayLayers(1);
    
    // TODO: We could use the swapchain image as transfer src/dst, but not every device SurfaceKHR supports this !?
    setImageUsage(vk::ImageUsageFlagBits::eColorAttachment); // | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage)
    setImageSharingMode(sharingMode);
    
    setQueueFamilyIndices(queueFamilyIndices);
    setPreTransform(swapchainTransform);
    setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    setPresentMode(swapchainPresentMode);
    setClipped(true);
    setOldSwapchain(oldSwapChain ? oldSwapChain->getHandle() : vk::SwapchainKHR(nullptr));
}

bool VulkanSwapchain::CreateInfo::isValid() const noexcept
{
    return imageExtent.width > 0 && imageExtent.height > 0;
}

//==============================================================================

VulkanSwapchain::VulkanSwapchain(const VulkanDevice& device_, const VulkanSurface& surface, const vk::SwapchainCreateInfoKHR& createInfo) : device(device_)
{
    const auto isSurfaceSupported = device.getPhysicalDevice().isSurfaceSupported(surface.getHandle());
    if (!isSurfaceSupported)
    {
        PW_DBG_V("Physical Device doesn't support the surface.");
        jassertfalse;
        return;
    }

    if (createInfo.imageFormat == vk::Format::eUndefined)
    {
        PW_DBG_V("Undefined swap chain image format.");
        jassertfalse;
        return;
    }

    if (createInfo.imageExtent.width == 0 || createInfo.imageExtent.height == 0)
    {
        PW_DBG_V("Invalid swap chain extent (0, 0).");
        jassertfalse;
        return;
    }
    
    vk::Result result;

    jassert(device.getHandle());
    std::tie(result, handle) = device.getHandle().createSwapchainKHRUnique(createInfo).asTuple();
    
    PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't create swapchain.");

    if (result == vk::Result::eSuccess)
    {
        surfaceFormat = createInfo.imageFormat;
        extent = createInfo.imageExtent;
        surfaceTransform = createInfo.preTransform;
        
        presentMode = createInfo.presentMode;

        PW_DBG_V("Created swap chain with "
            << "format = '" << vk::to_string(surfaceFormat.format) << "'"
            << ", color space = '" << vk::to_string(surfaceFormat.colorSpace) << "'"
            << ", size = '" << juce::String(extent.width) << " x " << juce::String(extent.height) << "'"
            << ", pre transform = '" << vk::to_string(surfaceTransform) << "'"
            << ", present mode = '" << vk::to_string(presentMode) << "'");

        getSwapchainImages();
    }
}

VulkanSwapchain::~VulkanSwapchain()
{
    PW_DBG_V("Destroyed swap chain.");
}

vk::Viewport VulkanSwapchain::getViewport() const noexcept
{
    vk::Viewport viewport
    (
        0.0f,
        0.0f,
        static_cast<float>(extent.width),
        static_cast<float>(extent.height),
        0.0f,
        1.0f
    );
        
    return viewport;
}

size_t VulkanSwapchain::getNumImages() const noexcept { return images.size(); }

vk::Image VulkanSwapchain::getImage(size_t index) const noexcept 
{ 
    jassert(index < images.size());
    return images[index];
}

void VulkanSwapchain::getSwapchainImages()
{
    vk::Result result;

    jassert(device.getHandle() && getHandle());
    std::tie(result, images) = device.getHandle().getSwapchainImagesKHR(getHandle());
            
    PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't get swapchain images.");

    if (result == vk::Result::eSuccess)
    {
        PW_DBG_V("Created swap chain images (count: " + juce::String(images.size()) + ").");
    }
}

vk::Result VulkanSwapchain::acquireNextImage(uint32_t& imageIndex, const VulkanSemaphore& signalSemaphore, juce::RelativeTime timeout) const noexcept
{
    const auto acquireTimeout = static_cast<uint64_t>(std::max<int64_t>(0, timeout.inMilliseconds())) * 1000; // Nano Seconds

    vk::Result result;

    jassert(device.getHandle() && getHandle());
    std::tie(result, imageIndex) = device.getHandle().acquireNextImageKHR(getHandle(), acquireTimeout, signalSemaphore.getHandle(), nullptr);

    return result;
}

vk::Result VulkanSwapchain::presentImage(uint32_t imageIndex, const VulkanSemaphore& waitSemaphore) const noexcept
{
    const auto& queue = getDevice().getGraphicsQueue();
        
    std::array<vk::Semaphore, 1> waits = { waitSemaphore.getHandle() };
    std::array<vk::SwapchainKHR, 1> swapchains = { getHandle() };
    std::array<uint32_t, 1> imageIndices = { imageIndex };

    auto presentInfo = vk::PresentInfoKHR()
        .setWaitSemaphores(waits)
        .setSwapchains(swapchains)
        .setImageIndices(imageIndices);

    jassert(queue.getHandle());
    return queue.getHandle().presentKHR(presentInfo);  
}

} // namespace parawave
