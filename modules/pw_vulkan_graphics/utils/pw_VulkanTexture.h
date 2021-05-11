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
class VulkanTexture final : public juce::ReferenceCountedObject
{
public:
    using Ptr = juce::ReferenceCountedObjectPtr<VulkanTexture>;

private:
    VulkanTexture() = delete;

public:
    VulkanTexture(const VulkanDevice& device, VulkanMemoryPool& memoryPool, uint32_t width_, uint32_t height_) :
        width(width_), height(height_),
        memoryImage(memoryPool, VulkanMemoryImage::CreateInfo(width, height, vk::Format::eB8G8R8A8Unorm)
            .setDeviceLocal().setSampled().setTransferDst()),
        imageView(device, memoryImage.getImage())   
    {
        //DBG("[Vulkan] Created cached image of size " << juce::String(width) << " x " << juce::String(height) << ".");
    }

    ~VulkanTexture()
    {
        //DBG("[Vulkan] Remove cached image of size " << juce::String(width) << " x " << juce::String(height) << ".");
    }

    const VulkanMemoryImage& getMemory() const noexcept { return memoryImage; }

    const VulkanImageView& getImageView() const noexcept { return imageView; }
        
    juce::Time getLastUsedTime() const noexcept { return lastUsed; }

    void setLastUsedTime(juce::Time newTime = juce::Time::getCurrentTime()) noexcept { lastUsed = newTime; }

    uint32_t getWidth() const noexcept { return width; }
    uint32_t getHeight() const noexcept { return height; }

    float getWidthProportion() const noexcept { return static_cast<float>(width) / static_cast<float>(memoryImage.getImage().getExtent().width); }
    float getHeightProportion() const noexcept { return static_cast<float>(height) / static_cast<float>(memoryImage.getImage().getExtent().height); }

    static VulkanTexture::Ptr get(const juce::Graphics& g, const juce::Image& image);

private:
    const uint32_t width;
    const uint32_t height;

    const VulkanMemoryImage memoryImage;
    const VulkanImageView imageView;

    juce::Time lastUsed;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanTexture)
};

//==============================================================================

} // namespace parawave