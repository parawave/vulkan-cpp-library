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
template <typename QuadQueueType>
struct EdgeTableRenderer
{
    forcedinline EdgeTableRenderer (QuadQueueType& q, juce::PixelARGB c) noexcept
        : quadQueue (q), colour (c) {}

    forcedinline void setEdgeTableYPos (int y) noexcept
    {
        currentY = y;
    }

    forcedinline void handleEdgeTablePixel (int x, int alphaLevel) const noexcept
    {
        auto c = colour;
        c.multiplyAlpha (alphaLevel);
        quadQueue.add (x, currentY, 1, 1, c);
    }

    forcedinline void handleEdgeTablePixelFull (int x) const noexcept
    {
        quadQueue.add (x, currentY, 1, 1, colour);
    }

    forcedinline void handleEdgeTableLine (int x, int width, int alphaLevel) const noexcept
    {
        auto c = colour;
        c.multiplyAlpha (alphaLevel);
        quadQueue.add (x, currentY, width, 1, c);
    }

    forcedinline void handleEdgeTableLineFull (int x, int width) const noexcept
    {
        quadQueue.add (x, currentY, width, 1, colour);
    }

    forcedinline void handleEdgeTableRectangle (int x, int y, int width, int height, int alphaLevel) const noexcept
    {
        auto c = colour;
        c.multiplyAlpha (alphaLevel);
        quadQueue.add (x, y, width, height, c);
    }

    forcedinline void handleEdgeTableRectangleFull (int x, int y, int width, int height) const noexcept
    {
        quadQueue.add (x, y, width, height, colour);
    }

private:
    QuadQueueType& quadQueue;
    const juce::PixelARGB colour;
    int currentY;

    JUCE_DECLARE_NON_COPYABLE (EdgeTableRenderer)
};

//==============================================================================
struct QuadQueue
{
    /** If you increase the size here, make sure the index buffer is also increased ! */
    enum { maxNumQuads = 1024 };

    using VertexType = ProgramHelpers::Vertex;

    //==============================================================================

    QuadQueue(const DeviceState& deviceState_, const VulkanCommandBuffer& commandBuffer_) : 
        deviceState(deviceState_), commandBuffer(commandBuffer_)
    {
        auto maxIndices = maxNumQuads * 6;

        auto numQuads = std::min (static_cast<int>(maxNumQuads), static_cast<int>(maxIndices) / 6);
        maxVertices = numQuads * 4 - 4;
    }

    ~QuadQueue()
    {
        reset();
    }

    void bindIndexBuffer() noexcept
    {
        commandBuffer.bindIndexBuffer(deviceState.memory.defaultQuadIndices.getBuffer());
    }

    template <typename IteratorType>
    void add (const IteratorType& et, juce::PixelARGB colour)
    {
        EdgeTableRenderer<QuadQueue> etr(*this, colour);
        et.iterate(etr);
    }

    void add(int x, int y, int w, int h, juce::PixelARGB colour) noexcept
    {
        jassert (w > 0 && h > 0);

        auto* v = vertexData + numVertices;

        v[0].x = v[2].x = static_cast<int16_t>(x);
        v[0].y = v[1].y = static_cast<int16_t>(y);
        v[1].x = v[3].x = static_cast<int16_t>(x + w);
        v[2].y = v[3].y = static_cast<int16_t>(y + h);

        // vk::Format::eA8B8G8R8UnormPack32
        auto rgba = static_cast<uint32_t>((colour.getAlpha() << 24) | (colour.getBlue() << 16) | (colour.getGreen() << 8) |  colour.getRed());

        v[0].colour = rgba;
        v[1].colour = rgba;
        v[2].colour = rgba;
        v[3].colour = rgba;

        numVertices += 4;

        if (numVertices > maxVertices)
            draw();
    }

    void flush() noexcept
    {
        if (numVertices > 0)
            draw();
    }

    void draw() noexcept
    {
        jassert(commandBuffer.getHandle());

        const auto createInfo = VulkanMemoryBuffer::CreateInfo()
            .setSize<VertexType>(numVertices).setHostVisible().setVertexBuffer();

        auto vertexBuffer = vertexBuffers.add(new VulkanMemoryBuffer(deviceState.memory.vertexPool, createInfo));
        vertexBuffer->write(vertexData, static_cast<vk::DeviceSize>(numVertices * sizeof(VertexType)));
        vertexBuffer->setDefragmentOnRelease(false);
        
        commandBuffer.bindVertexBuffer(vertexBuffer->getBuffer());
       
        const auto numIndices = static_cast<uint32_t>((numVertices * 3) / 2);
        commandBuffer.drawIndexed(numIndices);

        numVertices = 0;
    }

    void reset()
    {
        // Before we release all framebuffes we turn on defragmentation
        // With this we avoid a constantly growing memory allocations for vertex buffers

        for (auto& vertexBuffer : vertexBuffers)
            vertexBuffer->setDefragmentOnRelease(true);

        vertexBuffers.clearQuick(true);
    }

private:
    const DeviceState& deviceState;
    const VulkanCommandBuffer& commandBuffer;
    
    VertexType vertexData[maxNumQuads * 4];
    juce::OwnedArray<VulkanMemoryBuffer> vertexBuffers;

    int numVertices = 0;
    int maxVertices = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuadQueue)
};

//==============================================================================
struct GradientCache
{
    GradientCache(const DeviceState& deviceState_) noexcept : 
        deviceState(deviceState_), sampler(deviceState.device, VulkanSampler::CreateInfo().setFilter(vk::Filter::eLinear)) { }

    void reset()
    {
        descriptors.clearQuick(true);
        textures.clearQuick(true);

        gradientNeedsRefresh = true;
    }

    void triggerRefresh()
    {
        gradientNeedsRefresh = true;
    }

    SingleImageSamplerDescriptor* getTextureForGradient(const juce::ColourGradient& gradient)
    {
        if (gradientNeedsRefresh)
        {
            gradientNeedsRefresh = false;

            auto texture = textures.add(new LookupTexture(deviceState));
            texture->setGradient(gradient);

            auto descriptor = descriptors.add(new SingleImageSamplerDescriptor(deviceState.images.getImageSamplerDescriptorPool()));
            descriptor->update(*texture->view, sampler);

            /** TODO: Reuse descriptor set and maybe texures, just upload new gradient data into existing textures,
                since the size is always the same. */

            return descriptor;
        }

        return descriptors.getLast();
    }

private:
    struct LookupTexture
    {
        enum 
        { 
            numPixels = 256 
        };

        static constexpr auto defaultFormat = vk::Format::eB8G8R8A8Unorm;
        static constexpr auto lookupSize = static_cast<vk::DeviceSize>(numPixels * 4);
        
        LookupTexture(const DeviceState& deviceState)
        {
            const auto& device = deviceState.device;
            auto& pool = deviceState.memory.smallTexturePool;

            const auto imageCreateInfo = VulkanMemoryImage::CreateInfo(numPixels, 1, defaultFormat)
                .setDeviceLocal().setSampled().setTransferDst();

            texture.reset(new VulkanMemoryImage(pool, imageCreateInfo));
            view.reset(new VulkanImageView(device, texture->getImage()));

            const auto bufferCreateInfo = VulkanMemoryBuffer::CreateInfo()
                .setHostVisible().setTransferSrc().setSize(lookupSize);

            stagingBuffer.reset(new VulkanMemoryBuffer(pool, bufferCreateInfo));
            transfer.reset(new VulkanImageTransfer(device, texture->getImage(), *stagingBuffer));
        }

        ~LookupTexture()
        {
            texture->setDefragmentOnRelease(false);
            stagingBuffer->setDefragmentOnRelease(false);
        }

        void setGradient(const juce::ColourGradient& gradient) const
        {
            juce::PixelARGB lookup[numPixels];
            gradient.createLookupTable(lookup, numPixels);

            transfer->writePixels(lookup, lookupSize);
            transfer->copyBufferToImage();
        }

        std::unique_ptr<VulkanMemoryImage> texture;
        std::unique_ptr<VulkanImageView> view;

        // TODO: reset stagingBuffer & transfer after initial upload !
        std::unique_ptr<VulkanMemoryBuffer> stagingBuffer;
        std::unique_ptr<VulkanImageTransfer> transfer;
    };

private:
    enum { numTexturesToCache = 8, numGradientTexturesToCache = 10 };

    const DeviceState& deviceState;

    const VulkanSampler sampler;

    juce::OwnedArray<LookupTexture> textures;
    juce::OwnedArray<SingleImageSamplerDescriptor> descriptors;

    bool gradientNeedsRefresh = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GradientCache)
};

} // namespace parawave