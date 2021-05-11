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
 
namespace VulkanGraphicsContext
{

//==============================================================================
struct SavedState : public juce::RenderingHelpers::SavedStateBase<SavedState>
{
    using BaseClass = juce::RenderingHelpers::SavedStateBase<SavedState>;

    SavedState() = delete;

    explicit SavedState(FrameState* layer_) 
        : BaseClass(layer_->getBounds()), layer(layer_) 
    {
        layer->setTransformSource(&transform);
    }

    SavedState(const SavedState& other) 
        : BaseClass(other), font(other.font), layer(other.layer) 
    {
        layer->setTransformSource(&transform);
    }

    SavedState* beginTransparencyLayer(float opacity)
    {
        auto* s = new SavedState (*this);

        if (clip != nullptr)
        {
            // TODO : ? Complete any pending commands, so they are recorded before the new framebuffer draw call is added !
            // Maybe this is not necessary, since layer draw calls are chained with semaphores.

            // layer->flush();
            
            auto clipBounds = clip->getClipBounds();
            auto newLayer = layer->createRenderLayer(clipBounds);
          
            s->layer = newLayer;
            s->transparencyLayerAlpha = opacity;
            
            s->cloneClipIfMultiplyReferenced();

            newLayer->beginRender(true);
        }

        return s;
    }

    void endTransparencyLayer(SavedState& finishedLayerState)
    {
        if (clip != nullptr)
        {
            auto sourceLayer = finishedLayerState.layer;
            jassert(sourceLayer != nullptr);

            // sourceLayer->flush() ?
            
            sourceLayer->endRender();

            const auto result = sourceLayer->submit();
            if (result != vk::Result::eSuccess)
            {
                jassertfalse;
                return;
            }

            const auto alpha = static_cast<int>(finishedLayerState.transparencyLayerAlpha * 255.0f);

            const auto clipBounds = clip->getClipBounds();

            const auto x = clipBounds.getX();
            const auto y = clipBounds.getY();

            /** TODO: avoid dynamic_cast ? */ 

            /** The original code created an OpenGL image type here. This is unecessary, since the framebuffer
                can be used here. To avoid this, get the clip region as concrete type and pass it as iterator. */
            if (auto rectangleRegion = dynamic_cast<BaseClass::RectangleListRegionType*>(clip.get()))
            {
                layer->renderLayer(*rectangleRegion, *sourceLayer, alpha, x, y);
                return;
            }
            else if (auto edgeTableRegion = dynamic_cast<BaseClass::EdgeTableRegionType*>(clip.get()))
            {
                layer->renderLayer(edgeTableRegion->edgeTable, *sourceLayer, alpha, x, y);
                return;
            }
            else
            {
                // Clip region not implemented !
                jassertfalse;
            }
        }
    }

    using GlyphCacheType = juce::RenderingHelpers::GlyphCache<juce::RenderingHelpers::CachedGlyphEdgeTable<SavedState>, SavedState>;

    void drawGlyph(int glyphNumber, const juce::AffineTransform& trans)
    {
        if (clip != nullptr)
        {
            if (trans.isOnlyTranslation() && ! transform.isRotated)
            {
                auto& cache = GlyphCacheType::getInstance();
                juce::Point<float> pos (trans.getTranslationX(), trans.getTranslationY());

                if (transform.isOnlyTranslated)
                {
                    cache.drawGlyph (*this, font, glyphNumber, pos + transform.offset.toFloat());
                }
                else
                {
                    pos = transform.transformed (pos);

                    juce::Font f (font);
                    f.setHeight (font.getHeight() * transform.complexTransform.mat11);

                    auto xScale = transform.complexTransform.mat00 / transform.complexTransform.mat11;

                    if (std::abs (xScale - 1.0f) > 0.01f)
                        f.setHorizontalScale (xScale);

                    cache.drawGlyph (*this, f, glyphNumber, pos);
                }
            }
            else
            {
                auto fontHeight = font.getHeight();

                auto t = transform.getTransformWith (juce::AffineTransform::scale (fontHeight * font.getHorizontalScale(), fontHeight)
                                                                     .followedBy (trans));

                const std::unique_ptr<juce::EdgeTable> et (font.getTypeface()->getEdgeTableForGlyph (glyphNumber, t, fontHeight));

                if (et != nullptr)
                    fillShape (*new EdgeTableRegionType (*et), false);
            }
        }
    }

    juce::Rectangle<int> getMaximumBounds() const { return layer->getBounds(); }

    void setFillType(const juce::FillType& newFill)
    {
        BaseClass::setFillType(newFill);
        layer->refreshGradient();
    }

    template <typename IteratorType>
    void renderImageTransformed(IteratorType& iter, const juce::Image& src, int alpha,
                                const juce::AffineTransform& trans, juce::Graphics::ResamplingQuality quality, bool tiledFill) const
    {
        layer->renderImageTransformed(iter, src, alpha, trans, quality, tiledFill);
    }

    template <typename IteratorType>
    void renderImageUntransformed(IteratorType& iter, const juce::Image& src, int alpha, int x, int y, bool tiledFill) const
    {
        renderImageTransformed(iter, src, alpha, juce::AffineTransform::translation ((float) x, (float) y),
                               juce::Graphics::lowResamplingQuality, tiledFill);
    }

    template <typename IteratorType>
    void fillWithSolidColour(IteratorType& iter, juce::PixelARGB colour, bool /*replaceContents*/) const
    {
        layer->fillWithSolidColour(iter, colour);
    }

    template <typename IteratorType>
    void fillWithGradient(IteratorType& iter, juce::ColourGradient& gradient, const juce::AffineTransform& trans, bool /*isIdentity*/) const
    {
        layer->fillWithGradient(iter, fillType.colour.getPixelARGB(), gradient, trans);
    }

public:
    juce::Font font;
    RenderLayer* layer;

    RenderLayer* transparencyLayer = nullptr;

private:
    SavedState& operator= (const SavedState&) = delete;
};

//==============================================================================
struct FrameContext : public juce::RenderingHelpers::StackBasedLowLevelGraphicsContext<SavedState>
{
    FrameContext(FrameState& frame_) : frame(frame_)
    {
        stack.initialise(new SavedState(&frame));
    }

    VulkanRenderer* getRenderer() { return stack->layer; }

    /** We could override this to allow a custom physical pixel scale factor
    float getPhysicalPixelScaleFactor() override 
    { 
        return stack->transform.getPhysicalPixelScaleFactor();
        //return frame.getPhysicalPixelScaleFactor(); 
    }
    */

    FrameState& frame;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrameContext)
};

//==============================================================================
struct FrameContextImmediate : public juce::RenderingHelpers::StackBasedLowLevelGraphicsContext<SavedState>
{
    FrameContextImmediate(ImmediateFrameState& immediateState_) : 
        immediateState(immediateState_)
    {
        stack.initialise(new SavedState(&immediateState.frame));

        immediateState.startRender();
    }

    VulkanRenderer* getRenderer()  { return stack->layer; }

    ~FrameContextImmediate()
    {
        immediateState.flushRender();
    }

    ImmediateFrameState& immediateState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrameContextImmediate)
};

} // namespace VulkanGraphicsContext

//==============================================================================
std::unique_ptr<juce::LowLevelGraphicsContext> createVulkanGraphicsContext(FrameState& frame)
{
    return std::make_unique<VulkanGraphicsContext::FrameContext>(frame);
}

std::unique_ptr<juce::LowLevelGraphicsContext> createVulkanGraphicsContext(ImmediateFrameState& frame)
{
    return std::make_unique<VulkanGraphicsContext::FrameContextImmediate>(frame);
}

//==============================================================================
VulkanRenderer* VulkanRenderer::get(const juce::Graphics& g) noexcept
{
    auto& llContext = g.getInternalContext();

    if (auto frameContext = dynamic_cast<VulkanGraphicsContext::FrameContext*>(&llContext))
        return frameContext->getRenderer();
    else if (auto frameContextImmediate = dynamic_cast<VulkanGraphicsContext::FrameContextImmediate*>(&llContext))
        return frameContextImmediate->getRenderer();

    return nullptr;
}

VulkanTexture::Ptr VulkanTexture::get(const juce::Graphics& g, const juce::Image& image)
{
    if (auto renderer = VulkanRenderer::get(g))
        return renderer->getTextureFor(image);
 
    return VulkanTexture::Ptr();
}

} // namespace parawave