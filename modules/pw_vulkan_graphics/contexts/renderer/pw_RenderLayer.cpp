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
struct RenderCache
{
    RenderCache(DeviceState& deviceState_) :
        deviceState(deviceState_),
        gradientCache(deviceState)
    { }

    void reset()
    {
        gradientCache.reset();

        layers.clearQuick(true);

        textures.clearQuick();
        imageSamplerDescriptors.clearQuick(true);

        framebufferPixelData.clearQuick();
    }

    SingleImageSamplerDescriptor* createImageSamplerDescriptor()
    {
        return imageSamplerDescriptors.add(new SingleImageSamplerDescriptor(deviceState.images.getImageSamplerDescriptorPool()));
    }
        
    DeviceState& deviceState;
    GradientCache gradientCache;

    juce::OwnedArray<RenderLayer> layers;

    juce::ReferenceCountedArray<VulkanTexture> textures;
    juce::OwnedArray<SingleImageSamplerDescriptor> imageSamplerDescriptors;

    juce::ReferenceCountedArray<juce::ReferenceCountedObject> framebufferPixelData;
};

//==============================================================================
class RenderLayer : public RenderFrame
{
public:
    class PixelData : public juce::ImagePixelData
    {
    private:
        PixelData() = delete;
        
    public:
        using juce::ImagePixelData::ImagePixelData;
        
        virtual ~PixelData() = default;
        
        virtual RenderLayer& getRenderLayer() = 0;
    };
    
private:
    struct ImageInfo
    {
        uint32_t width;
        uint32_t height;

        float widthProportion;
        float heightProportion;
    };

public:
    RenderLayer(const DeviceState& deviceState, uint32_t width, uint32_t height, vk::Format format) :
        RenderFrame(deviceState, width, height, format), quadQueue(deviceState, commandBuffer)
    {}

    ~RenderLayer() override = default;

    void initialiseBindings() override
    {
        quadQueue.bindIndexBuffer();
    }

    void resetBindings() override
    {
        quadQueue.flush();
    }

    void setSamplerQuality(juce::Graphics::ResamplingQuality newQuality)
    {
        if (newQuality != currentQuality)
        {
            currentQuality = newQuality;
            currentSampler = &state.images.getSampler(newQuality);
        }
    }

    //==============================================================================
    template <typename IteratorType>
    void renderImageTransformed(IteratorType& iter, const juce::Image& src, int alpha, const juce::AffineTransform& transform, juce::Graphics::ResamplingQuality quality, bool tiledFill)
    { 
        /* If the image uses the VulkanImageType pixel data we can directly use the framebuffer instead 
           of uploading it as texture first. */
        if (auto pixelData = dynamic_cast<PixelData*>(src.getPixelData()))
        {
            /** The image of the framebuffer pixel data could be created on the stack,
                so the data must be stored until this frame is completed. Hold onto a reference ptr! */
            cache->framebufferPixelData.add(pixelData);

            const auto& frameAttachment = pixelData->getRenderLayer().getAttachment();
            const auto& image = frameAttachment.memoryImage.getImage();

            const auto& imageExtent = image.getExtent();

            setSamplerQuality(quality);
            
            const auto descriptor = cache->createImageSamplerDescriptor();
            descriptor->update(frameAttachment.imageView, *currentSampler);

            ImageInfo info;

            info.width = imageExtent.width;
            info.height = imageExtent.height;
            info.widthProportion = 1.0f;
            info.heightProportion = 1.0f;
             
            // It's necessary to draw the framebuffer flipped !
            setShaderForTiledImageFill(transform, info, descriptor->getDescriptorSet(), tiledFill, true);
        }
        else
        {
            auto texture = cache->textures.add(state.images.getTextureFor(src));

            ImageInfo info;

            info.width = texture->getWidth();
            info.height = texture->getHeight();
            info.widthProportion = texture->getWidthProportion();
            info.heightProportion = texture->getHeightProportion();

            const auto descriptor = state.images.getTextureDescriptor(*texture, quality);

            setShaderForTiledImageFill(transform, info, descriptor->getDescriptorSet(), tiledFill);
        }

        const auto a = static_cast<uint8_t>(alpha);
        const juce::PixelARGB colour(a, a, a, a);

        //const auto noRotation = transform.mat01 == 0.0 && transform.mat10 == 0.0f;
        
        quadQueue.add(iter, colour);
        quadQueue.flush();
    }

    void setShaderForTiledImageFill(const juce::AffineTransform& transform, const ImageInfo& info, const VulkanDescriptorSet& imageDescriptorSet, bool isTiledFill, bool flipY = false)
    {
        if (isTiledFill)
        {
            bindPipeline(state.pipelines.tiledImage.pipeline);
            bindDescriptorSet(state.pipelines.tiledImage.pipelineLayout, imageDescriptorSet);
        }
        else
        {
            bindPipeline(state.pipelines.image.pipeline);
            bindDescriptorSet(state.pipelines.image.pipelineLayout, imageDescriptorSet); 
        }

        const auto fillBounds = getBounds().toFloat();
        setImageMatrix(transform, info, fillBounds.getX(), fillBounds.getY(), isTiledFill, flipY);
    }

    void setImageMatrix(const juce::AffineTransform& transform, const ImageInfo& info, float targetX, float targetY, bool isTiledFill, bool flipY = false)
    {
        using Parameters = ImagePushConstants;

        Parameters values;

        values.set2DBounds(getBounds().toFloat());

        values.setMatrix(transform, info.width, info.height, info.widthProportion, info.heightProportion, targetX, targetY, isTiledFill, flipY);

        if (isTiledFill)
            commandBuffer.pushConstants(state.pipelines.tiledImage.pipelineLayout, &values, sizeof(Parameters));
        else
            commandBuffer.pushVertexConstants(state.pipelines.image.pipelineLayout, &values, sizeof(Parameters));
    }

    //==============================================================================
    template <typename IteratorType>
    void fillWithSolidColour(IteratorType& iter, juce::PixelARGB colour)
    {
        bindPipeline(state.pipelines.solidColour.pipeline);

        set2DBounds();

        quadQueue.add(iter, colour);
        quadQueue.flush();
    }

    void set2DBounds()
    {
        using Parameters = ProgramHelpers::GraphicsPipelineCreateInfo::PushConstants;

        Parameters values;
        values.screenBounds.set(getBounds().toFloat());

        commandBuffer.pushVertexConstants(state.pipelines.solidColour.pipelineLayout, &values, sizeof(Parameters));
    }

    //==============================================================================
    template <typename IteratorType>
    void fillWithGradient(IteratorType& iter, juce::PixelARGB colour, juce::ColourGradient& gradient, const juce::AffineTransform& transform)
    {
        const auto renderArea = getBounds().toFloat();

        auto t = transform.translated (0.5f - renderArea.getX(),
                                       0.5f - renderArea.getY());
        
        auto p1 = gradient.point1.transformedBy (t);
        auto p2 = gradient.point2.transformedBy (t);
        auto p3 = juce::Point<float> (gradient.point1.x + (gradient.point2.y - gradient.point1.y),
                                      gradient.point1.y - (gradient.point2.x - gradient.point1.x)).transformedBy (t);

        const VulkanPipelineLayout* descriptorPipelineLayout = nullptr;

        if (gradient.isRadial)
        {
            bindPipeline(state.pipelines.radialGradient.pipeline);
            
            // Push Contstants
            {
                using Parameters = RadialGradientPushConstants;

                Parameters values;

                values.set2DBounds(renderArea);
                values.setMatrix(p1, p2, p3);

                commandBuffer.pushConstants(state.pipelines.radialGradient.pipelineLayout, &values, sizeof(Parameters));
            }

            descriptorPipelineLayout = &state.pipelines.radialGradient.pipelineLayout;
        }
        else
        {
            p1 = juce::Line<float> (p1, p3).findNearestPointTo (p2);
            
            const juce::Point<float> delta (p2.x - p1.x, p1.y - p2.y);

            float grad;
            float length;

            const auto isGradient1 = std::abs(delta.x) < std::abs(delta.y);
            if (isGradient1)
            {
                grad = delta.x / delta.y;
                length = (p2.y - grad * p2.x) - (p1.y - grad * p1.x);
            }
            else
            {
                grad = delta.y / delta.x;
                length = (p2.x - grad * p2.y) - (p1.x - grad * p1.y);
            }

            const auto& pipeline = isGradient1 ? state.pipelines.linearGradient1.pipeline :
                                                 state.pipelines.linearGradient2.pipeline;

            const auto& pipelineLayout = isGradient1 ? state.pipelines.linearGradient1.pipelineLayout : 
                                                       state.pipelines.linearGradient2.pipelineLayout;

            bindPipeline(pipeline);

            // Push Contstants
            {
                using Parameters = LinearGradientPushConstants;

                Parameters values;

                values.set2DBounds(renderArea);
                values.setGradient(p1, grad, length);

                commandBuffer.pushConstants(pipelineLayout, &values, sizeof(Parameters));
            }

            descriptorPipelineLayout = &pipelineLayout;
        }

        if (auto gradientTexture = cache->gradientCache.getTextureForGradient(gradient))
        {
            commandBuffer.bindDescriptorSet(*descriptorPipelineLayout, gradientTexture->getDescriptorSet());

            quadQueue.add(iter, colour);
            quadQueue.flush();
        }
        else
        {
            // Couldn't create cached gradient texture
            jassertfalse;
        }
    }

    void refreshGradient()
    {
        cache->gradientCache.triggerRefresh();
    }

    RenderLayer* createRenderLayer(const juce::Rectangle<int>& frameArea)
    {
        const auto width = static_cast<uint32_t>(frameArea.getWidth());
        const auto height = static_cast<uint32_t>(frameArea.getHeight());

        jassert(width > 0 && height > 0);

        const auto framebufferFormat = getAttachment().memoryImage.getImage().getFormat();
        jassert(framebufferFormat != vk::Format::eUndefined);

        auto layer = cache->layers.add(new RenderLayer(state, width, height, framebufferFormat));

        // Since the new layer will render to a buffer at position zero, the bound must be offset by the current clip area
        const auto newBounds = layer->getBounds().withPosition(frameArea.getPosition());
        layer->setBounds(newBounds);

        // The current layer must wait for all commands of the new layer
        layer->setWaitSemaphore(getWaitSemaphore());
        setWaitSemaphore(&layer->getCompletedSemaphore());

        // The new layer will use the same frame cache for intermediate allocations
        layer->setCache(cache);

        return layer;
    }

    template <typename IteratorType>
    void renderLayerTransformed(IteratorType& iter, const RenderLayer& src, int alpha, const juce::AffineTransform& transform)
    {
        const auto layerBounds = src.getBounds();
        
        // Create texture info, so the layer framebuffer is handled like a regular texture image
        ImageInfo info;

        info.width = static_cast<uint32_t>(layerBounds.getWidth());
        info.height = static_cast<uint32_t>(layerBounds.getHeight());
        info.widthProportion = 1.0f;
        info.heightProportion = 1.0f;

        // Create a Texture Descriptor and set it to the FrameLayer framebuffer image view
        const auto descriptor = cache->createImageSamplerDescriptor();
        descriptor->update(src.getAttachment().imageView, state.images.getSampler(juce::Graphics::mediumResamplingQuality));

        setShaderForTiledImageFill(transform, info, descriptor->getDescriptorSet(), false, true);

        const auto a = static_cast<uint8_t>(alpha);

        quadQueue.add(iter, juce::PixelARGB(a, a, a, a));
        quadQueue.flush();
    }

    template <typename IteratorType>
    void renderLayer(IteratorType& iter, const RenderLayer& src, int alpha, int x, int y)
    {
        renderLayerTransformed(iter, src, alpha, juce::AffineTransform::translation(static_cast<float>(x), static_cast<float>(y)));
    }

    //==============================================================================
    void renderFramebuffer(const VulkanImageView& imageView, const juce::Rectangle<int>& renderArea)
    {
        const auto transform = juce::AffineTransform::translation(static_cast<float>(renderArea.getX()), static_cast<float>(renderArea.getY()));

        ImageInfo info;

        info.width = static_cast<uint32_t>(renderArea.getWidth());
        info.height = static_cast<uint32_t>(renderArea.getHeight());
        info.widthProportion = 1.0f;
        info.heightProportion = 1.0f;

        const auto descriptor = cache->createImageSamplerDescriptor();
        descriptor->update(imageView, state.images.getSampler(juce::Graphics::mediumResamplingQuality));

        setShaderForTiledImageFill(transform, info, descriptor->getDescriptorSet(), false, true);

        const auto a = static_cast<uint8_t>(255);

        quadQueue.add(renderArea.getX(), renderArea.getY(), renderArea.getWidth(), renderArea.getHeight(), juce::PixelARGB(a, a, a, a));
        quadQueue.flush();
    }

protected:
    void setCache(RenderCache* newCache) noexcept
    {
        jassert(newCache);
        cache = newCache;

        currentQuality = juce::Graphics::ResamplingQuality::mediumResamplingQuality;
        currentSampler = &state.images.getSampler(currentQuality);
    }

protected:
    QuadQueue quadQueue;

    juce::Graphics::ResamplingQuality currentQuality = juce::Graphics::ResamplingQuality::mediumResamplingQuality;
    const VulkanSampler* currentSampler = nullptr;

private:
    RenderCache* cache = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RenderLayer)
};

} // namespace parawave
