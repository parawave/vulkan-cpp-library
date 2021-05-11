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
    VulkanMemory

    A fixed block of device memory that can be used to acquire/dispose memory 
    ranges. Optionally mapped if the MemoryPropertyFlag is specified 
    as 'eHostVisible'. 
*/
class VulkanMemory final
{
private:
    enum
    {
        minNumRanges = 16
    };

public:
    VulkanMemory(const VulkanDevice& _device, vk::DeviceSize memorySize_, uint32_t memoryTypeIndex_) :
        device(_device), memory(_device, vk::MemoryAllocateInfo(memorySize_, memoryTypeIndex_)), memorySize(memorySize_), memoryTypeIndex(memoryTypeIndex_)
    {
        ranges.add(VulkanMemoryRange(*this, memorySize, 0, true));

        const auto& physicalDevice = device.getPhysicalDevice();
        
        // If the used memory type is host visible the device memory, map the entire range!
        const auto memoryTypeProperty = physicalDevice.getMemoryProperties().memoryTypes[memoryTypeIndex].propertyFlags;
        if ((memoryTypeProperty & vk::MemoryPropertyFlagBits::eHostVisible) == vk::MemoryPropertyFlagBits::eHostVisible)
        {
            vk::Result result;

            jassert(device.getHandle() && memory.getHandle());
            std::tie(result, data) = device.getHandle().mapMemory(memory.getHandle(), 0, VK_WHOLE_SIZE);

            PW_CHECK_VK_RESULT_SUCCESS(result, "Failed to map device memory.");
        }

        PW_DBG_V("Allocated " + juce::File::descriptionOfSizeInBytes(memorySize) + " (" + juce::String(memorySize) + " bytes) device memory. Type: " 
            << vk::to_string(memoryTypeProperty));
    }

    ~VulkanMemory()
    {
        if (isHostVisible())
        {
            jassert(device.getHandle() && memory.getHandle());
            device.getHandle().unmapMemory(memory.getHandle());
        }

        const auto& physicalDevice = device.getPhysicalDevice();
        const auto memoryTypeProperty = physicalDevice.getMemoryProperties().memoryTypes[memoryTypeIndex].propertyFlags;

        PW_DBG_V("Deallocated " + juce::File::descriptionOfSizeInBytes(memorySize) + " (" + juce::String(memorySize) + " bytes) device memory. Type: " 
            << vk::to_string(memoryTypeProperty));
    }

    vk::DeviceSize size() const noexcept { return memorySize; }

    const VulkanDeviceMemory& getDeviceMemory() const noexcept { return memory; }

    uint32_t getMemoryTypeIndex() const noexcept { return memoryTypeIndex; }

    void* getData() const noexcept { return data; }

    bool isHostVisible() const noexcept { return getData() != nullptr; }

    bool contains(const VulkanMemoryRange& range) const noexcept { return ranges.contains(range); }

    bool isFree() const noexcept 
    {
        for (auto& range : ranges)
        {
            if (!range.isFree())
                return false;
        }

        return true;
    }

    bool acquireRange(VulkanMemoryRange& destRange, vk::DeviceSize rangeSize, vk::DeviceSize rangeAlignment)
    {
        // Block too small for the requested range size
        if(rangeSize > memorySize)
            return false;

        int index;
        vk::DeviceSize newSize;

        std::tie(index, newSize) = findFreeRange(rangeSize, rangeAlignment);

        if (index == -1 || newSize == 0)
            return false;

        {
            auto& range = ranges.getReference(index);

            range.memorySize = newSize;

            // Move offset to the required alignment
            if(rangeAlignment != 0)
            {
                const auto delta = range.memoryOffset % rangeAlignment;
                if (delta != 0)
                    range.memoryOffset += rangeAlignment - delta;
            }

            // If it's a perfect fit, just return the range
            if(range.memorySize == rangeSize) 
            {
                range.free = false;
                destRange = range;

                return true;
            }
        }

        vk::DeviceSize newRangeSize;
        vk::DeviceSize newRangeOffset;

        // If the requested range is smaller than the found range, split it up!
        {
            auto& range = ranges.getReference(index);

            newRangeSize = range.memorySize - rangeSize;
            newRangeOffset = range.memoryOffset + rangeSize;

            range.memorySize = rangeSize;
            range.free = false;

            destRange = range;
        }

        ranges.add(VulkanMemoryRange(*this, newRangeSize, newRangeOffset));

        return true;
    }

    void disposeRange(const VulkanMemoryRange& range, bool defragmentAfter = true) noexcept
    {
        auto iterator(std::find(ranges.begin(), ranges.end(), range));
        jassert(iterator != ranges.end());

        iterator->free = true;

        if(defragmentAfter)
            defragmentRanges();
    }

    void defragmentRanges()
    {
        // Quick Merge: If all ranges are free, merge all into one!
        if (mergeAllRanges())
            return;

        sortRanges();

        //DBG("Before Fragmentation:");
        //printRanges();

        mergeAdjacentRanges();
        removeEmptyRanges();
        
        if (ranges.size() > 0)
        {
            // First : Move offset to left of the allocation
            {
                auto& range = ranges.getReference(0);
                if (range.isFree())
                    range.memoryOffset = 0;
            }
            
            // Last : Increase size so the range fits the max allocation size
            {
                auto& range = ranges.getReference(ranges.size() - 1);
                if (range.isFree() && range.getEnd() < memorySize)
                    range.memorySize = memorySize - range.getOffset();
            }
        }

        //DBG("After Fragmentation:");
        //printRanges();

        /** 
            TODO 
            If there are unassigned memory ranges due to alignment offset shift,
            these could cause fragmentation in the long term run. 
            
            Resize free ranges to the start of the next range!
        */
    }

private:
    struct RangeComparator
    {
        static int compareElements(const VulkanMemoryRange& first, const VulkanMemoryRange& second) noexcept
        {
            if (first.getOffset() < second.getOffset())
                return -1;
            else if (first.getOffset() > second.getOffset())
                return 1;

            return 0;
        }
    };

    // Sort the Ranges by Offset
    void sortRanges()
    {
        const RangeComparator comparator;
        ranges.sort(comparator);
    }

    void removeEmptyRanges()
    {
        ranges.removeIf ([](const VulkanMemoryRange& range) { return range.isEmpty(); });
    }

    // Merge adjacent free ranges into one, zero size the remaining range
    void mergeAdjacentRanges()
    {
        for (int i = ranges.size(); --i >= 0;)
        {
            if (i - 1 < 0)
                continue;

            auto& right = ranges.getReference(i);
            if (!right.isFree())
                continue;

            auto& left = ranges.getReference(i - 1);
            if (!left.isFree())
                continue;

            const auto distance = right.getEnd() - left.getEnd();
               
            left.memorySize += distance;

            right.memoryOffset = 0;
            right.memorySize = 0;
        }
    }

    bool mergeAllRanges()
    {
        for (const auto& range : ranges)
        {
            if (!range.isFree())
                return false;
        }

        ranges.clearQuick();
        ranges.add(VulkanMemoryRange(*this, memorySize, 0, true));

        return true;
    }

    std::pair<int, vk::DeviceSize> findFreeRange(vk::DeviceSize rangeSize, vk::DeviceSize rangeAlignment) const noexcept
    {
        int i = 0;

        for (auto& range : ranges)
        {
            if (range.isFree())
            {
                auto newSize = range.getSize();

                // If there is an alignment requirement, the offset shifts and the available size will be reduced
                if(rangeAlignment != 0)
                {
                    const auto delta = range.getOffset() % rangeAlignment;
                    if (delta != 0)
                        newSize -= rangeAlignment - delta;
                }
               
                // Is the (due to alignment) reduced size still big enough?
                if (newSize >= rangeSize)
                    return std::make_pair(i, newSize);
            }

            ++i;
        }

        return std::make_pair(-1, vk::DeviceSize(0));
    }

    void printRanges()
    {
        DBG("Memory Ranges: ");

        for (int i = 0; i < ranges.size(); ++i)
        {
            DBG("Range [" << (ranges[i].isFree() ? " " : "X") << "] [offset = " << ranges[i].getOffset() << ", size = " << ranges[i].getSize() << "]");
        }

        DBG("Total [offset = " << 0 << ", size = " << memorySize << "]");
    }

private:
    const VulkanDevice& device;
    const VulkanDeviceMemory memory;

    const vk::DeviceSize memorySize;
    const uint32_t memoryTypeIndex;

    void* data = nullptr;

    juce::Array<VulkanMemoryRange, juce::DummyCriticalSection, minNumRanges> ranges;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanMemory)
};

} // namespace parawave