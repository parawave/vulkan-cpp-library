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
   
namespace
{

//==============================================================================
struct CachedPipelines : public juce::ReferenceCountedObject
{
    using Ptr = juce::ReferenceCountedObjectPtr<CachedPipelines>;

    CachedPipelines(VulkanDevice& device, const CachedImages& images, const CachedRenderPasses& renderPasses) :
        singleImageSamplerLayout(images.getImageSamplerDescriptorPool().layout),
        solidColour(device, renderPasses.offscreen),
        linearGradient1(device, singleImageSamplerLayout, renderPasses.offscreen),
        linearGradient2(device, singleImageSamplerLayout, renderPasses.offscreen),
        radialGradient(device, singleImageSamplerLayout, renderPasses.offscreen),
        image(device, singleImageSamplerLayout, renderPasses.offscreen),
        tiledImage(device, singleImageSamplerLayout, renderPasses.offscreen),
        overlay(device, singleImageSamplerLayout, renderPasses.swapchain)
    {}

    static CachedPipelines* get(VulkanDevice& device, const CachedImages& images, const CachedRenderPasses& renderPasses)
    {
        static constexpr char objectID[] = "CachedPipelines";
        auto pipelines = static_cast<CachedPipelines*>(device.getAssociatedObject (objectID));
        if (pipelines == nullptr)
        {
            pipelines = new CachedPipelines(device, images, renderPasses);
            device.setAssociatedObject(objectID, pipelines);
        }

        return pipelines;
    }

private:
    const VulkanDescriptorSetLayout& singleImageSamplerLayout;

public:
    SolidColourProgram solidColour;
    LinearGradientProgram1 linearGradient1;
    LinearGradientProgram2 linearGradient2;
    RadialGradientProgram radialGradient;
    ImageProgram image;
    TiledImageProgram tiledImage;
    OverlayProgram overlay;
};

} // namespace

} // namespace parawave