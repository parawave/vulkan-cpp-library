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

template<size_t MB>
struct MemorySizeHelper
{
    static constexpr int getNumBytes() { return static_cast<vk::DeviceSize>(MB * 1024 * 1024); }
};

//==============================================================================
struct CachedMemory : public juce::ReferenceCountedObject
{
    using Ptr = juce::ReferenceCountedObjectPtr<CachedMemory>;

private:
    static constexpr int defaultPoolSize = MemorySizeHelper<16>::getNumBytes(); // 16 MB

    static constexpr auto smallPoolSize = MemorySizeHelper<4>::getNumBytes(); // 4 MB
    static constexpr auto mediumPoolSize = MemorySizeHelper<8>::getNumBytes(); // 8 MB
    static constexpr auto bigPoolSize = MemorySizeHelper<16>::getNumBytes(); // 16 MB

    static constexpr auto defaultNumIndices = 1024 * 6; // uint16 indices for 1024 quads

public:
    CachedMemory() = delete;

    explicit CachedMemory(const VulkanDevice& device) : 
        stagingPool(device, defaultPoolSize),
        smallTexturePool(device, smallPoolSize),
        mediumTexturePool(device, mediumPoolSize),
        bigTexturePool(device, bigPoolSize),
        framebufferPool(device, bigPoolSize),
        vertexPool(device, smallPoolSize),
        defaultQuadIndices(vertexPool, VulkanMemoryBuffer::CreateInfo()
            .setSize<uint16_t>(defaultNumIndices).setDeviceLocal().setIndexBuffer().setTransferDst()) 
    {
       VulkanIndexBuffer<uint16_t>::generateQuadrilateralIndices(defaultQuadIndices, device, vertexPool, defaultNumIndices);
    }

    void minimizeStorage(bool forceMinimize = false)
    {
        const auto time = juce::Time::getCurrentTime();

        const auto duration = time - lastStorageCheck;
        if (forceMinimize || duration.inSeconds() > 4.0)
        {
            stagingPool.minimizeStorage();

            smallTexturePool.minimizeStorage();
            mediumTexturePool.minimizeStorage();
            bigTexturePool.minimizeStorage();

            framebufferPool.minimizeStorage();

            vertexPool.minimizeStorage();
            
            lastStorageCheck = time;
        }
    }

    juce::String getSizeInBytes(const VulkanMemoryPool& pool)
    {
        return juce::File::descriptionOfSizeInBytes(pool.size());
    }

    void printUsage()
    {
        PW_DBG_V("[Vulkan] Cached Memory :"
            << "\n\tStaging Pool: " << getSizeInBytes(stagingPool)
            << "\n\tTexture Pool (Small): " << getSizeInBytes(smallTexturePool)
            << "\n\tTexture Pool (Medium): " << getSizeInBytes(mediumTexturePool)
            << "\n\tTexture Pool (Big): " << getSizeInBytes(bigTexturePool)
            << "\n\tFramebuffer Pool: " << getSizeInBytes(framebufferPool)
            << "\n\tVertex Pool: " << getSizeInBytes(vertexPool));
    } 

    static CachedMemory* get(VulkanDevice& device)
    {
        static constexpr char objectID[] = "CachedMemory";
        auto memory = static_cast<CachedMemory*>(device.getAssociatedObject (objectID));
        if (memory == nullptr)
        {
            memory = new CachedMemory(device);
            device.setAssociatedObject(objectID, memory);
        }

        return memory;
    }

    VulkanMemoryPool stagingPool;

    VulkanMemoryPool smallTexturePool;
    VulkanMemoryPool mediumTexturePool;
    VulkanMemoryPool bigTexturePool;

    VulkanMemoryPool framebufferPool;

    VulkanMemoryPool vertexPool;

    /** Most of the time triangles are draw as quads with an indices buffer. To save the recreation time,
        cache a default buffer on device local memory here. */
    VulkanMemoryBuffer defaultQuadIndices;

private:
    juce::Time lastStorageCheck;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CachedMemory)
};

} // namespace

} // namespace parawave
