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
/** Helper class to generate static index buffer arrays. */
template<typename IndexType>
class VulkanIndexBuffer final
{
public:
    static void fillQuadrilateralIndices(IndexType* indices, IndexType numIndices)
    {
        constexpr auto indicesPerQuad = 6;
        constexpr auto verticesPerQuad = 4;

        // [0 1 2 1 2 3]
        for (IndexType i = 0, v = 0; i < numIndices; i += indicesPerQuad, v += verticesPerQuad)
	    {
		    indices[i] = static_cast<IndexType>(v);
		    indices[i + 1] = indices[i + 3] = static_cast<IndexType>(v + 1);
		    indices[i + 2] = indices[i + 4] = static_cast<IndexType>(v + 2);
		    indices[i + 5] = static_cast<IndexType>(v + 3);
	    }
    }

    /** Generates indices for quad drawing. The supplied destination buffer must be device local.
        A staging buffer will be used to transfer the generated indices.*/
    static void generateQuadrilateralIndices(const VulkanMemoryBuffer& dest, const VulkanDevice& device, VulkanMemoryPool& pool, IndexType numIndices)
    {
        juce::HeapBlock<IndexType> indices(numIndices);

        auto data = indices.getData();
        const auto dataSize = static_cast<size_t>(numIndices * sizeof(IndexType));

        fillQuadrilateralIndices(data, numIndices);
        writeWithStagingBuffer(dest, device, pool, data, dataSize);
    }

    static void writeWithStagingBuffer(const VulkanMemoryBuffer& dest, const VulkanDevice& device, VulkanMemoryPool& pool, const void* dataSrc, size_t dataSrcSize)
    {
        const auto sourceSize = static_cast<vk::DeviceSize>(dataSrcSize);
        
        const VulkanMemoryBuffer stagingBuffer(pool, VulkanMemoryBuffer::CreateInfo(sourceSize).setHostVisible().setTransferSrc());
        stagingBuffer.write(dataSrc, dataSrcSize);

        VulkanBufferTransfer transfer(device, dest.getBuffer(), stagingBuffer.getBuffer());
        
        transfer.writeToBuffer();
        transfer.waitForFence();
    }
};

} // namespace parawave