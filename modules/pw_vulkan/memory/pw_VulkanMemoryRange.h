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

class VulkanMemory;
   
//==============================================================================
/** 
    VulkanMemoryRange

    A range of memory in a Vulkan DeviceMemory allocation. 
*/
class VulkanMemoryRange final
{
    friend class VulkanMemory;

public:
    VulkanMemoryRange() : memory(nullptr), memorySize(0), memoryOffset(0), free(false) {}

    explicit VulkanMemoryRange(const VulkanMemory& memory_, vk::DeviceSize memorySize_, vk::DeviceSize memoryOffset_ = 0, bool free_ = true) noexcept :
        memory(&memory_), memorySize(memorySize_), memoryOffset(memoryOffset_), free(free_) {}

    ~VulkanMemoryRange() = default;

    VulkanMemoryRange(const VulkanMemoryRange& other) noexcept :
        memory(other.memory), memorySize(other.memorySize), memoryOffset(other.memoryOffset), free(other.free) {}

    VulkanMemoryRange(VulkanMemoryRange&& other) noexcept :
        memory(nullptr), memorySize(0), memoryOffset(0), free(false)
    {
       *this = std::move(other);
    }

    VulkanMemoryRange& operator=(const VulkanMemoryRange& other) noexcept
    {
        if (this != &other)
        {
            memory = other.memory;
            memorySize = other.memorySize;
            memoryOffset = other.memoryOffset;

            free = other.free;
        }

        return *this;
    }

    VulkanMemoryRange& operator=(VulkanMemoryRange&& other) noexcept
    {
        if (this != &other)
        {
            memory = other.memory;
            memorySize = other.memorySize;
            memoryOffset = other.memoryOffset;

            free = other.free;

            other.memory = nullptr;
            other.memorySize = 0;
            other.memoryOffset = 0;

            other.free = false;
        }

        return *this;
    }

    bool operator==(const VulkanMemoryRange& other) const noexcept
    {
        if (memory == other.memory && getSize() == other.getSize() && 
            getOffset() == other.getOffset() && isFree() == other.isFree())
            return true;

        return false;
    }

    const VulkanMemory* getMemoryBlock() const noexcept { return memory; }

    vk::DeviceSize getSize() const noexcept { return memorySize; }

    vk::DeviceSize getOffset() const noexcept { return memoryOffset; }

    vk::DeviceSize getEnd() const noexcept { return getOffset() + getSize(); }

    bool isFree() const noexcept { return free; }

    bool isEmpty() const noexcept { return getSize() == vk::DeviceSize(0); }

private:
    const VulkanMemory* memory;

    vk::DeviceSize memorySize;
    vk::DeviceSize memoryOffset;

    bool free;
};

} // namespace parawave
