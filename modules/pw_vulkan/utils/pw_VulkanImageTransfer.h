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
    VulkanImageTransfer
    
    Copy from host visible into device local memory (or vice versa) using a 
    staging buffer and transition into a shader read only layout. 
*/
class VulkanImageTransfer : public VulkanCommandSequence
{
public:
    struct CopyRegion : public vk::BufferImageCopy
    {
    public:
        CopyRegion(const VulkanImage& image)
        {
            const auto imageSubresourceLayers = vk::ImageSubresourceLayers()
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setMipLevel(0)
                .setBaseArrayLayer(0)
                .setLayerCount(1);

            setBufferOffset(0);
            setBufferRowLength(0);
            setBufferImageHeight(0);
            setImageSubresource(imageSubresourceLayers);
            setImageOffset(vk::Offset3D());
            setImageExtent(image.getExtent());
        }
    };

public:
    VulkanImageTransfer(const VulkanDevice& device_, const VulkanImage& image_, const VulkanMemoryBuffer& stagingMemory_) :
        VulkanCommandSequence(device_), image(image_), stagingMemory(stagingMemory_)
    { 
        // Image transfers are currently only supported for B G R A, 8 bit per pixel data
        jassert(image.getFormat() == vk::Format::eB8G8R8A8Unorm);
    }

    ~VulkanImageTransfer() override = default;

    void readPixels(void* dataDst, vk::DeviceSize dataDstSize) const
    {
        const auto copySize = std::min(stagingMemory.getMemorySize(), dataDstSize);
        std::memcpy(dataDst, stagingMemory.getData(), static_cast<size_t>(copySize));
    }

    /** Write BGRA pixels into the host visible staging buffer. */
    void writePixels(const void* dataSrc, vk::DeviceSize dataSrcSize) const
    {
        stagingMemory.write(dataSrc, dataSrcSize); 
    }

    void readBitmapData(const juce::Image::BitmapData& bitmapData) const noexcept 
    { 
        // TODO
        jassertfalse;
        ignoreUnused(bitmapData);
    }

    /** Write a JUCE bitmap data into the host visible staging buffer. */
    void writeBitmapData(const juce::Image::BitmapData& bitmapData) const noexcept
    {
        const auto imageW = bitmapData.width;
        const auto imageH = bitmapData.height;

        const auto maxSize = static_cast<vk::DeviceSize>(imageW * imageH * 4);

        switch (bitmapData.pixelFormat)
        {
            case juce::Image::ARGB:
            {
                juce::HeapBlock<juce::PixelARGB> dataCopy;
                copyPixels<juce::PixelARGB>(dataCopy, bitmapData.data, bitmapData.lineStride, imageW, imageH);
                writePixels(dataCopy.getData(), maxSize); 
                break;
            }
            case juce::Image::RGB:
            {
                juce::HeapBlock<juce::PixelARGB> dataCopy;
                copyPixels<juce::PixelRGB>(dataCopy, bitmapData.data, bitmapData.lineStride, imageW, imageH);
                writePixels(dataCopy.getData(), maxSize);
                break;
            }
            case juce::Image::SingleChannel:
            {
                juce::HeapBlock<juce::PixelARGB> dataCopy;
                copyPixels<juce::PixelAlpha>(dataCopy, bitmapData.data, bitmapData.lineStride, imageW, imageH);
                writePixels(dataCopy.getData(), maxSize);
                break;
            }
            case juce::Image::UnknownFormat:
                PW_DBG_V("Format for juce::Image not implemented!");
                jassertfalse; 
            default: 
                break;
        }
    }

    void readImage(const juce::Image& imageDest) const noexcept
    {
        const juce::Image::BitmapData bitmapData(imageDest, juce::Image::BitmapData::writeOnly);
        readBitmapData(bitmapData);
    }

    void writeImage(const juce::Image& imageSource) const noexcept
    {
        const juce::Image::BitmapData bitmapData(imageSource, juce::Image::BitmapData::readOnly);
        writeBitmapData(bitmapData);
    }

    void copyBufferToImage(const vk::BufferImageCopy& region)
    {
        submit([&](const VulkanCommandBuffer& cb)
        {
            cb.transitionImageLayout(image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
            cb.copyBufferToImage(image, stagingMemory.getBuffer(), region);
            cb.transitionImageLayout(image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
        }, true);
    }

    void copyBufferToImage()
    {
        const CopyRegion region(image);
        copyBufferToImage(region);
    }

    void copyImageToBuffer(const vk::BufferImageCopy& region)
    {
        submit([&](const VulkanCommandBuffer& cb)
        {
            cb.transitionImageLayout(image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferSrcOptimal);
            cb.copyImageToBuffer(stagingMemory.getBuffer(), image, region);
            cb.transitionImageLayout(image, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
        }, true);
    }

    void copyImageToBuffer()
    {
        const CopyRegion region(image);
        copyImageToBuffer(region);
    }

private:
    template <class PixelType>
    static void copyPixels(juce::HeapBlock<juce::PixelARGB>& dataCopy, const uint8_t* srcData, const int lineStride, const int w, const int h)
    {
        dataCopy.malloc(w * h);

        for (int y = 0; y < h; ++y)
        {
            auto* src = reinterpret_cast<const PixelType*>(srcData);
            auto* dst = reinterpret_cast<juce::PixelARGB*>(dataCopy + w * y);

            for (int x = 0; x < w; ++x)
                dst[x].set(src[x]);

            srcData += lineStride;
        }
    }

private:
    const VulkanImage& image;
    const VulkanMemoryBuffer& stagingMemory;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanImageTransfer)
};

} // namespace parawave