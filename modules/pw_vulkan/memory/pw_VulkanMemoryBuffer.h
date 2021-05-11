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
    VulkanMemoryBuffer
*/
class VulkanMemoryBuffer final
{
private:
    VulkanMemoryBuffer() = delete;

public:
    struct CreateInfo
    {
        CreateInfo(vk::DeviceSize bufferSize_ = {}, vk::BufferUsageFlags bufferUsage_ = {}, vk::MemoryPropertyFlags memoryProperties_ = {}) : 
            bufferSize(bufferSize_), bufferUsage(bufferUsage_), memoryProperties(memoryProperties_) {}

        CreateInfo& setSize(vk::DeviceSize bufferSize_) noexcept { bufferSize = bufferSize_; return *this; }

        template<typename DataType>
        CreateInfo& setSize(size_t numElements) noexcept { setSize(static_cast<vk::DeviceSize>(numElements * sizeof(DataType))); return *this; }

        CreateInfo& setUsage(vk::BufferUsageFlags bufferUsage_) noexcept { bufferUsage = bufferUsage_; return *this; }

        CreateInfo& setMemoryProperties(vk::MemoryPropertyFlags memoryProperties_) noexcept { memoryProperties = memoryProperties_; return *this; }

        CreateInfo& setVertexBuffer() noexcept { bufferUsage |= vk::BufferUsageFlagBits::eVertexBuffer; return *this; }

        CreateInfo& setIndexBuffer() noexcept { bufferUsage |= vk::BufferUsageFlagBits::eIndexBuffer; return *this; }

        CreateInfo& setTransferDst() noexcept { bufferUsage |= vk::BufferUsageFlagBits::eTransferDst; return *this; }

        CreateInfo& setTransferSrc() noexcept { bufferUsage |= vk::BufferUsageFlagBits::eTransferSrc; return *this; }

        CreateInfo& setHostVisible() noexcept
        {
            memoryProperties |= vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent; return *this;
        }

        CreateInfo& setDeviceLocal() noexcept
        {
            memoryProperties |= vk::MemoryPropertyFlagBits::eDeviceLocal; return *this;
        }

        vk::DeviceSize bufferSize = {};
        vk::BufferUsageFlags bufferUsage = {};
        vk::MemoryPropertyFlags memoryProperties = {};
    };

public:
    VulkanMemoryBuffer(VulkanMemoryPool& pool_, const vk::BufferCreateInfo& bufferCreateInfo, vk::MemoryPropertyFlags memoryProperties) : 
        pool(pool_), buffer(pool_.getDevice(), bufferCreateInfo), memoryRange(pool_.acquire(buffer.getMemoryRequirements(), memoryProperties))
    {
        if (auto memoryBlock = memoryRange.getMemoryBlock())
        {
            const auto& device = pool.getDevice();

            const auto& deviceMemory = memoryBlock->getDeviceMemory();
            jassert(deviceMemory.getHandle());

            jassert(memoryRange.getSize() >= buffer.getSize());

            jassert(device.getHandle() && buffer.getHandle());
            auto result = device.getHandle().bindBufferMemory(buffer.getHandle(), deviceMemory.getHandle(), memoryRange.getOffset());

            PW_CHECK_VK_RESULT_SUCCESS(result, "Failed to bind device memory for buffer.");
        }
        else
        {
            jassertfalse;
        }
    }

    VulkanMemoryBuffer(VulkanMemoryPool& pool_, vk::DeviceSize bufferSize, vk::BufferUsageFlags bufferUsage, vk::MemoryPropertyFlags memoryProperties) : 
        VulkanMemoryBuffer(pool_, vk::BufferCreateInfo().setSize(bufferSize).setUsage(bufferUsage).setSharingMode(vk::SharingMode::eExclusive), memoryProperties) {}

    VulkanMemoryBuffer(VulkanMemoryPool& pool_, CreateInfo createInfo) :
        VulkanMemoryBuffer(pool_, createInfo.bufferSize, createInfo.bufferUsage, createInfo.memoryProperties) {}

    ~VulkanMemoryBuffer()
    {
        pool.dispose(memoryRange, defragmentFlag);
    }

    const VulkanBuffer& getBuffer() const noexcept { return buffer; }

    vk::DeviceSize getMemorySize() const noexcept { return memoryRange.getSize(); }

    bool useDefragmentOnRelease() const noexcept { return defragmentFlag; }
    void setDefragmentOnRelease(bool defragmentOnRelease) noexcept { defragmentFlag = defragmentOnRelease; }

    bool isHostVisible() const noexcept { return getData() != nullptr; }

    // Get the host visible memory address of the requested range. Will return nullptr if it's unmapped.
    void* getData() const noexcept 
    { 
        if(const auto memoryBlock = memoryRange.getMemoryBlock())
            if (memoryBlock->isHostVisible())
                return reinterpret_cast<uint8_t*>(memoryBlock->getData()) + memoryRange.getOffset();

        return nullptr;
    }

    void write(const void* dataSrc, vk::DeviceSize dataSrcSize) const noexcept
    {
        if (isHostVisible())
        {
            jassert(dataSrcSize <= memoryRange.getSize());
            std::memcpy(getData(), dataSrc, static_cast<size_t>(dataSrcSize));
        }
    }

private:
    VulkanMemoryPool& pool;

    const VulkanBuffer buffer;
    const VulkanMemoryRange memoryRange;

    bool defragmentFlag = true;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanMemoryBuffer)
};

} // namespace parawave