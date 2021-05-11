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
    VulkanBufferView
*/
class VulkanBufferView final
{
public:
    struct CreateInfo : public vk::BufferViewCreateInfo
    {
        CreateInfo(const VulkanBuffer& buffer, const vk::Format& format)
        {
            setBuffer(buffer.getHandle());
            setFormat(format);
            setOffset(0);
            setRange(buffer.getSize());
        }
    };

public:
    VulkanBufferView(const VulkanDevice& device, const vk::BufferViewCreateInfo& createInfo)
    {
        vk::Result result;
    
        jassert(device.getHandle());
        std::tie(result, handle) = device.getHandle().createBufferViewUnique(createInfo).asTuple();

        PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't create buffer view.");
    }

    VulkanBufferView(const VulkanDevice& device, const VulkanBuffer& buffer, const vk::Format& format = vk::Format::eUndefined)
        : VulkanBufferView(device, CreateInfo(buffer, format)) {}

    ~VulkanBufferView() = default;

    const vk::BufferView& getHandle() const noexcept { return *handle; }

private:
    vk::UniqueBufferView handle;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanBufferView)
};

} // namespace parawave