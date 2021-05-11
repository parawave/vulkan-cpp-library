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
    VulkanSampler

    objects represent the state of an image sampler which is used by the 
    implementation to read image data and apply filtering and other 
    transformations for the shader.
*/
class VulkanSampler
{
public:
    struct CreateInfo : public vk::SamplerCreateInfo
    {
        CreateInfo()
        {
            setMagFilter(vk::Filter::eLinear);
            setMinFilter(vk::Filter::eLinear);
            setMipmapMode(vk::SamplerMipmapMode::eLinear);
            setAddressModeU(vk::SamplerAddressMode::eClampToEdge);
            setAddressModeV(vk::SamplerAddressMode::eClampToEdge);
            setAddressModeW(vk::SamplerAddressMode::eClampToEdge);
            setMipLodBias(0.0f);
            setAnisotropyEnable(false);
            setMaxAnisotropy(1.0f);
            setCompareEnable(false);
            setCompareOp(vk::CompareOp::eNever);
            setMinLod(0.0f);
            setMaxLod(0.0f);
            setBorderColor(vk::BorderColor::eFloatTransparentBlack);
            setUnnormalizedCoordinates(false);
        }

        CreateInfo& setFilter(vk::Filter filter_) noexcept 
        { 
            setMagFilter(filter_); 
            setMinFilter(filter_); 
            return *this; 
        }

        CreateInfo& setAddressMode(vk::SamplerAddressMode addressMode_) noexcept 
        { 
            setAddressModeU(addressMode_);
            setAddressModeV(addressMode_);
            setAddressModeW(addressMode_);
            return *this; 
        }
    };

public:
    VulkanSampler(const VulkanDevice& device, const vk::SamplerCreateInfo& createInfo)
    {
        vk::Result result;
    
        jassert(device.getHandle());
        std::tie(result, handle) = device.getHandle().createSamplerUnique(createInfo).asTuple();

        PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't create sampler.");
    }

    VulkanSampler(const VulkanDevice& device)
        : VulkanSampler(device, CreateInfo()) {}
    
    ~VulkanSampler() = default;

    const vk::Sampler& getHandle() const noexcept { return *handle; }

private:
    vk::UniqueSampler handle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanSampler)
};

} // namespace parawave