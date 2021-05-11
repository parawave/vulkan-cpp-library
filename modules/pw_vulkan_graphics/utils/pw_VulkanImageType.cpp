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
  
//==============================================================================
class VulkanPixelData : public RenderLayer::PixelData
{
private:
    static constexpr auto defaultFormat = vk::Format::eB8G8R8A8Unorm;

public:
    VulkanPixelData(const VulkanContext& context_, int w, int h, bool shouldClearImage) : 
        PixelData(juce::Image::ARGB, w, h),
        context(context_),
        state(context, static_cast<uint32_t>(width), static_cast<uint32_t>(height), shouldClearImage, defaultFormat),
        pixelStride(4), lineStride(width * pixelStride)
    {}

    std::unique_ptr<juce::LowLevelGraphicsContext> createLowLevelContext() override
    {
        sendDataChangeMessage();
        return createVulkanGraphicsContext(state);
    }

    std::unique_ptr<juce::ImageType> createType() const override { return std::make_unique<VulkanImageType>(context); }

    ImagePixelData::Ptr clone() override
    {
        std::unique_ptr<VulkanPixelData> im (new VulkanPixelData (context, width, height, false)); 
        // Don't need to clear the image, since we immediately overwrite it !

        juce::Image newImage(im.release());

        juce::Graphics g(newImage);
        g.drawImageAt(juce::Image(*this), 0, 0, false);

        return ImagePixelData::Ptr(newImage.getPixelData());
    }

    void initialiseBitmapData(juce::Image::BitmapData& bitmapData, int x, int y, juce::Image::BitmapData::ReadWriteMode mode) override
    {
        bitmapData.pixelFormat = pixelFormat;
        bitmapData.lineStride  = lineStride;
        bitmapData.pixelStride = pixelStride;

        switch (mode)
        {
            case juce::Image::BitmapData::writeOnly:
                DataReleaser<Dummy,  Writer>::initialise(state, bitmapData, x, y);
                break;
            case juce::Image::BitmapData::readOnly:
                DataReleaser<Reader, Dummy>::initialise(state, bitmapData, x, y);
                break;
            case juce::Image::BitmapData::readWrite:
                DataReleaser<Reader, Writer>::initialise(state, bitmapData, x, y);
                break;
            default:
                jassertfalse; 
                break;
        }

        if (mode != juce::Image::BitmapData::readOnly)
            sendDataChangeMessage();
    }
    
    RenderLayer& getRenderLayer() override
    {
        return state.frame;
    }

    const VulkanContext& context;
    
    ImmediateFrameState state;

private:
    int pixelStride;
    int lineStride;

    struct Dummy
    {
        Dummy(ImmediateFrameState&, int, int, int, int) noexcept {}
        static void read(ImmediateFrameState&, juce::Image::BitmapData&, int, int) noexcept {}
        static void write(const juce::PixelARGB*) noexcept {}
    };

    struct Reader
    {
        static void read(ImmediateFrameState& state, juce::Image::BitmapData& bitmapData, int x, int y)
        {
            const auto& memoryImage = state.frame.getAttachment().memoryImage;
            const auto& image = memoryImage.getImage();

            const auto copySize = vk::DeviceSize(bitmapData.width * bitmapData.height * sizeof(juce::PixelARGB));

            // Vulkan staged Image to Buffer transfer
            {
                const auto bufferCreateInfo = VulkanMemoryBuffer::CreateInfo()
                .setHostVisible().setTransferDst().setSize(copySize);

                const VulkanMemoryBuffer stagingBuffer(state.deviceState.memory.stagingPool, bufferCreateInfo);
                VulkanImageTransfer transfer(state.deviceState.device, image, stagingBuffer);

                // Copy Region
                {
                    VulkanImageTransfer::CopyRegion region(image);

                    region.setImageOffset(vk::Offset3D(x, y, 0));
                    region.setImageExtent(vk::Extent3D(bitmapData.width, bitmapData.height, 1));

                    transfer.copyImageToBuffer(region);
                }
                
                transfer.waitForFence();

                transfer.readPixels(bitmapData.data, copySize);
            }

            verticalRowFlip ((juce::PixelARGB*) bitmapData.data, bitmapData.width, bitmapData.height);
        }

        static void verticalRowFlip(juce::PixelARGB* const data, const int w, const int h)
        {
            juce::HeapBlock<juce::PixelARGB> tempRow (w);
            auto rowSize = (size_t) w * sizeof (juce::PixelARGB);

            for (int y = 0; y < h / 2; ++y)
            {
                juce::PixelARGB* const row1 = data + y * w;
                juce::PixelARGB* const row2 = data + (h - 1 - y) * w;
                
                std::memcpy (tempRow, row1, rowSize);
                std::memcpy (row1, row2, rowSize);
                std::memcpy (row2, tempRow, rowSize);
            }
        }
    };

    struct Writer
    {
        Writer (ImmediateFrameState& state_, int x, int y, int w, int h) noexcept
            : state(state_), area (x, y, w, h) {}
        
        void write (const juce::PixelARGB* const data) const noexcept
        {
            juce::HeapBlock<juce::PixelARGB> invertedCopy(area.getWidth() * area.getHeight());
            auto rowSize = (size_t) area.getWidth() * sizeof (juce::PixelARGB);

            for (int y = 0; y < area.getHeight(); ++y)
                std::memcpy (invertedCopy + area.getWidth() * y,
                        data + area.getWidth() * (area.getHeight() - 1 - y), rowSize);

            const auto& memoryImage = state.frame.getAttachment().memoryImage;
            const auto& image = memoryImage.getImage();

            const auto copySize = vk::DeviceSize(area.getWidth() * area.getHeight() * sizeof(juce::PixelARGB));

            // Vulkan staged Buffer to Image transfer
            {
                const auto bufferCreateInfo = VulkanMemoryBuffer::CreateInfo()
                .setHostVisible().setTransferSrc().setSize(copySize);

                const VulkanMemoryBuffer stagingBuffer(state.deviceState.memory.stagingPool, bufferCreateInfo);
                VulkanImageTransfer transfer(state.deviceState.device, image, stagingBuffer);

                transfer.writePixels(invertedCopy, copySize);
                
                // Copy Region
                {
                    VulkanImageTransfer::CopyRegion region(image);

                    region.setImageOffset(vk::Offset3D(area.getX(), area.getY(), 0));
                    region.setImageExtent(vk::Extent3D(area.getWidth(), area.getHeight(), 1));

                    transfer.copyBufferToImage(region);
                }

                transfer.waitForFence();
            }
        }

        ImmediateFrameState& state;
        const juce::Rectangle<int> area;

        JUCE_DECLARE_NON_COPYABLE (Writer)
    };

    template <class ReaderType, class WriterType>
    struct DataReleaser  : public juce::Image::BitmapData::BitmapDataReleaser
    {
        DataReleaser(ImmediateFrameState& state, int x, int y, int w, int h)
            : data((size_t) (w * h)),
              writer(state, x, y, w, h)
        {}

        ~DataReleaser()
        {
            writer.write (data);
        }

        static void initialise(ImmediateFrameState& state, juce::Image::BitmapData& bitmapData, int x, int y)
        {
            auto* r = new DataReleaser(state, x, y, bitmapData.width, bitmapData.height);
            bitmapData.dataReleaser.reset(r);

            bitmapData.data = (uint8_t*) r->data.get();
            bitmapData.lineStride = (bitmapData.width * bitmapData.pixelStride + 3) & ~3;

            ReaderType::read(state, bitmapData, x, y);
        }

        juce::HeapBlock<juce::PixelARGB> data;
        WriterType writer;
    };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanPixelData)
};

//==============================================================================
VulkanImageType::VulkanImageType(const VulkanContext& context_) : context(context_) {}
VulkanImageType::~VulkanImageType() = default;

juce::ImagePixelData::Ptr VulkanImageType::create (juce::Image::PixelFormat, int width, int height, bool shouldClearImage) const
{
    // VulkanContext must be initialised before it's used !
    jassert(context.getDevice());

    return context.getDevice() != nullptr ? 
        std::make_unique<VulkanPixelData>(context, width, height, shouldClearImage).release() : juce::ImagePixelData::Ptr();
}

int VulkanImageType::getTypeID() const
{
    return static_cast<int>(PW_VULKAN_IMAGE_TYPE_ID);
}

} // namespace parawave
