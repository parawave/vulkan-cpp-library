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
/** 
    SingleImageSamplerDescriptorPool 

    A descriptor set pool for a single combined image sampler, usable as uniform
    in a fragment shader. 
*/
class SingleImageSamplerDescriptorPool final
{
private:
    struct DescriptorSetLayoutInfo : public vk::DescriptorSetLayoutCreateInfo
    {
        DescriptorSetLayoutInfo() { setBindings(bindings); }

        std::array<vk::DescriptorSetLayoutBinding, 1> bindings =
        {
            vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr)
        };
    };

    struct DescriptorPoolInfo : public vk::DescriptorPoolCreateInfo
    {
        DescriptorPoolInfo(uint32_t maxSets)
        {
            for (auto& poolSize : poolSizes)
                poolSize.setDescriptorCount(maxSets);

            setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
            setMaxSets(maxSets);
            setPoolSizes(poolSizes);
        }

        std::array<vk::DescriptorPoolSize, 1> poolSizes =
        {
            vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler)
        };
    };

public:
    SingleImageSamplerDescriptorPool(const VulkanDevice& device, uint32_t maxSets) :
        layout(device, DescriptorSetLayoutInfo()), createInfo(maxSets),
        pool(device, layout, createInfo) {}

    const VulkanDescriptorSetLayout layout;
    const DescriptorPoolInfo createInfo;
    
    VulkanDescriptorSetPool pool;
};

//==============================================================================
/** 
    SingleImageSamplerDescriptor
*/
class SingleImageSamplerDescriptor final
{
private:
    SingleImageSamplerDescriptor() = delete;

public:
    SingleImageSamplerDescriptor(SingleImageSamplerDescriptorPool& imageSamplerDescriptors) : 
        descriptor(imageSamplerDescriptors.pool) {}

    ~SingleImageSamplerDescriptor() = default;

    const VulkanDescriptorSet& getDescriptorSet() const noexcept { return descriptor.getDescriptorSet(); }

    void update(const VulkanImageView& imageView, const VulkanSampler& sampler) const noexcept
    {
        auto imageInfo = vk::DescriptorImageInfo()
            .setSampler(sampler.getHandle())
            .setImageView(imageView.getHandle())
            .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

        std::array<vk::WriteDescriptorSet, 1> descriptorWrites;

        // Image Sampler
        {
            auto& descriptorWrite = descriptorWrites[0];
            
            descriptorWrite
                .setDstSet(descriptor.getDescriptorSet().getHandle())
                .setDstBinding(0)
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                .setPImageInfo(&imageInfo);
        }

        descriptor.updateDescriptorSet(descriptorWrites.data(), static_cast<uint32_t>(descriptorWrites.size()));
    }

private:
    const VulkanDescriptor descriptor;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SingleImageSamplerDescriptor)
};

//==============================================================================

namespace
{

struct CachedImages : public juce::ReferenceCountedObject
{
    enum
    {
        defaultDescriptorPoolSize = 256
    };

    using Ptr = juce::ReferenceCountedObjectPtr<CachedImages>;

    CachedImages() = delete;

    CachedImages(const VulkanDevice& device_, CachedMemory& memory_) : 
        device(device_), 
        lowQualitySampler(device, VulkanSampler::CreateInfo().setFilter(vk::Filter::eNearest)),
        mediumQualitySampler(device, VulkanSampler::CreateInfo().setFilter(vk::Filter::eLinear)),
        highQualitySampler(device, VulkanSampler::CreateInfo().setFilter(vk::Filter::eLinear)),
        copySampler(device, VulkanSampler::CreateInfo().setFilter(vk::Filter::eNearest).setAddressMode(vk::SamplerAddressMode::eClampToBorder)),
        memory(memory_), imageSamplerDescriptorPool(device, defaultDescriptorPoolSize) {}

    ~CachedImages()
    {
        // Remove Texture Samplers before deleting Textures
        samplerByTexture.clear();
        textureSamplers.clear();
    }
    
    static CachedImages* get(VulkanDevice& device, CachedMemory& memory)
    {
        static constexpr char objectID[] = "CachedImages";
        auto images = static_cast<CachedImages*>(device.getAssociatedObject (objectID));
        if (images == nullptr)
        {
            images = new CachedImages(device, memory);
            device.setAssociatedObject(objectID, images);
        }

        return images;
    }

    const SingleImageSamplerDescriptorPool& getImageSamplerDescriptorPool() const noexcept { return imageSamplerDescriptorPool; }

    SingleImageSamplerDescriptorPool& getImageSamplerDescriptorPool() noexcept { return imageSamplerDescriptorPool; }

    const VulkanSampler& getCopySampler() const noexcept { return copySampler; }

    const VulkanSampler& getSampler(juce::Graphics::ResamplingQuality quality) const noexcept
    {
        switch (quality)
        {
            case juce::Graphics::ResamplingQuality::lowResamplingQuality:
                return lowQualitySampler;
            default:
            case juce::Graphics::ResamplingQuality::mediumResamplingQuality:
                return mediumQualitySampler;
            case juce::Graphics::ResamplingQuality::highResamplingQuality:
                return highQualitySampler;
        }
    }

    //==============================================================================
    /** If any render pass is using the texture, make sure to reference it until the rendering is completed ! */
    VulkanTexture::Ptr getTextureFor(const juce::Image& image)
    {
        cleanCollections();

        auto pixelData = image.getPixelData();

        if (!collectionByPixelData.contains(pixelData))
        {
            auto collection = collections.add(new TextureCollection(*this, pixelData));
            collectionByPixelData.set(pixelData, collection);
        }

        if (auto collection = collectionByPixelData[pixelData])
            return collection->getTexture(image);

        jassertfalse;
        return nullptr;
    }

    const SingleImageSamplerDescriptor* getTextureDescriptor(const VulkanTexture& texture, juce::Graphics::ResamplingQuality quality)
    {
        auto textureSampler = getTextureSampler(texture);
        jassert(textureSampler);

        return textureSampler->getDescriptor(quality);
    }

private:
    //==============================================================================
    class TextureSampler final
    {
    private:
        TextureSampler() = delete;

    public:
        TextureSampler(CachedImages& owner, const VulkanImageView& imageView) :
            lqDescriptor(owner.imageSamplerDescriptorPool),
            mqDescriptor(owner.imageSamplerDescriptorPool),
            hqDescriptor(owner.imageSamplerDescriptorPool)
        {
            lqDescriptor.update(imageView, owner.lowQualitySampler);
            mqDescriptor.update(imageView, owner.mediumQualitySampler);
            hqDescriptor.update(imageView, owner.highQualitySampler);
        }

        const SingleImageSamplerDescriptor* getDescriptor(juce::Graphics::ResamplingQuality quality) const noexcept
        {
            switch (quality)
            {
                case juce::Graphics::ResamplingQuality::lowResamplingQuality:
                    return &lqDescriptor;
                default:
                case juce::Graphics::ResamplingQuality::mediumResamplingQuality:
                    return &mqDescriptor;
                case juce::Graphics::ResamplingQuality::highResamplingQuality:
                    return &hqDescriptor;
            }
        }

    private:
        SingleImageSamplerDescriptor lqDescriptor;
        SingleImageSamplerDescriptor mqDescriptor;
        SingleImageSamplerDescriptor hqDescriptor;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextureSampler)
    };

    //==============================================================================
    const TextureSampler* getTextureSampler(const VulkanTexture& texture)
    {
        if (!samplerByTexture.contains(&texture))
        {
            auto textureSampler = textureSamplers.add(new TextureSampler(*this, texture.getImageView()));
            samplerByTexture.set(&texture, textureSampler);
        }

        return samplerByTexture[&texture];
    }

    void disposeTextureSampler(const VulkanTexture& texture)
    {
        if (samplerByTexture.contains(&texture))
        {
            auto textureSampler = samplerByTexture[&texture];
            textureSamplers.removeObject(textureSampler);

            samplerByTexture.remove(&texture);
        }
    }

    //==============================================================================
    struct TextureCollection;

    /** This will add the texture collection to a disposed list, what means 
        it won't be used for the creation of new textures. */
    void disposeCollection(TextureCollection& collection)
    {
        jassert(collection.pixelData != nullptr);

        collectionByPixelData.remove(collection.pixelData);
        disposedCollections.add(&collection);
    }

    //==============================================================================
    /** It's possible that multiple variation of a ImagePixelData* exist !
        If the data is changed during rendering while beeing used in another render pass, 
        we have to make sure the image in GPU memory is not freed yet. */
    struct TextureCollection final : private juce::ImagePixelData::Listener
    {
    private:
        enum
        {
            smallImageSize = 64 * 64,
            mediumImageSize = 512 * 512,
        };

        VulkanMemoryPool& getTexturePool(CachedMemory& cachedMemory, uint32_t width_, uint32_t height_) const noexcept
        {   
            const auto totalSize = width_ * height_;

            if (totalSize <= smallImageSize)
                return cachedMemory.smallTexturePool;
            else if(totalSize <= mediumImageSize)
                return cachedMemory.mediumTexturePool;
            else 
                return cachedMemory.bigTexturePool;
        }

    public:
        TextureCollection(CachedImages& owner_, juce::ImagePixelData* pixelData_) : 
            owner(owner_), pixelData(pixelData_)
        {
            pixelData->listeners.add(this);
        }

        ~TextureCollection()
        {
            if (pixelData != nullptr)
                pixelData->listeners.remove(this);

            for (auto texture : textures)
                owner.disposeTextureSampler(*texture);
        }

        /** If a texture is only referenced once, it's in this collection and if no 
            textures are referenced, the whole collection can be considered unused. */
        bool isUnused() const noexcept
        {
            for (auto texture : textures)
            {
                const auto count = texture->getReferenceCount();
                if (count > 1)
                    return false;
            }

            return true;
        }

        void clean()
        {
            removeCompletedTransfers();
            removeUnusedTextures();
        }

        void removeCompletedTransfers()
        {
            for (int i = transfers.size(); --i >= 0;)
            {
                if (transfers[i]->getCompletedFence().wait())
                {
                    transfers.remove(i);
                    stagingBuffers.remove(i);
                }
            }
        }

        void removeUnusedTextures()
        {
            for (int i = textures.size(); --i >= 0;)
            {
                if (auto texture = textures[i].get())
                {
                    /** If the texture is only referenced by the collection, it is not used anymore 
                        But don't remove the last texture, since can be used in the next render call */
                    if (texture->getReferenceCount() == 1) //&& textures.size() > 1)
                    {
                        /** Let the texture remain in memory for at least a few seconds, to avoid lags
                            when quickly switching between different views */
                        const auto duration = owner.currentTime - texture->getLastUsedTime();
                        if (duration.inSeconds() > 1.0)
                        {
                            owner.disposeTextureSampler(*texture);

                            textures.remove(i);
                        }  
                    }
                }
            }

            if (textures.size() == 0)
                needReloading = true;
        }

        VulkanMemoryBuffer* createStagingBuffer(vk::DeviceSize bufferSize)
        {
            const auto createInfo = VulkanMemoryBuffer::CreateInfo()
                .setHostVisible().setTransferSrc().setSize(bufferSize);

            return stagingBuffers.add(new VulkanMemoryBuffer(owner.memory.stagingPool, createInfo));
        }
        
        VulkanTexture::Ptr getTexture(const juce::Image& image)
        {
            VulkanTexture::Ptr texture;

            if (needReloading)
            {
                const auto w = static_cast<uint32_t>(image.getWidth());
                const auto h = static_cast<uint32_t>(image.getHeight());

                auto& memoryPool = getTexturePool(owner.memory, w, h);

                texture = textures.add(new VulkanTexture(owner.device, memoryPool, w, h));

                auto stagingBuffer = createStagingBuffer(vk::DeviceSize(w * h * 4));
                auto& imageBuffer = texture->getMemory();

                auto transfer = transfers.add(new VulkanImageTransfer(owner.device, imageBuffer.getImage(), *stagingBuffer));                
                
                transfer->writeImage(image);
                transfer->copyBufferToImage();

                needReloading = false;
            }
            else
            {
                texture = textures.getLast();
            }

            /** If there is no cached texture and it was not uploaded,
                there's definitely something wrong here ! */
            jassert(texture);

            if(texture)
                texture->setLastUsedTime();

            return texture;
        }

        //==============================================================================
        void imageDataChanged(juce::ImagePixelData* newPixelData) override
        {
            jassert(newPixelData == pixelData);
            ignoreUnused(newPixelData);
            
            needReloading = true;
        }

        void imageDataBeingDeleted(juce::ImagePixelData* /*newPixelData*/) override
        {
            owner.disposeCollection(*this);

            if (pixelData != nullptr)
            {
                pixelData->listeners.remove(this);
                pixelData = nullptr;
            }
        }

        CachedImages& owner;
        juce::ImagePixelData* pixelData;

        juce::ReferenceCountedArray<VulkanTexture> textures;
        
        juce::OwnedArray<VulkanMemoryBuffer> stagingBuffers;
        juce::OwnedArray<VulkanImageTransfer> transfers;

        juce::Time lastUsed;

        bool needReloading = true;
    };

    //==============================================================================
    /** The texture collections can't be deleted immediately, check if any texure is still referencing it. */
    void cleanCollections()
    {
        currentTime = juce::Time::getCurrentTime();

        for(auto& collection : collections)
            collection->clean();

        for (int i = disposedCollections.size(); --i >= 0;)
        {
            auto collection = disposedCollections[i];
            if (collection->isUnused())
            {
                collection = disposedCollections.removeAndReturn(i);
                collections.removeObject(collection);
            }
        } 
    }
private:
    const VulkanDevice& device;

    const VulkanSampler lowQualitySampler;
    const VulkanSampler mediumQualitySampler;
    const VulkanSampler highQualitySampler; // TODO: Activate Sampler Anisotropy if available ?

    const VulkanSampler copySampler;

    CachedMemory& memory;
    SingleImageSamplerDescriptorPool imageSamplerDescriptorPool;

    juce::OwnedArray<TextureSampler> textureSamplers;
    juce::HashMap<const VulkanTexture*, TextureSampler*> samplerByTexture;

    juce::OwnedArray<TextureCollection> collections;
    juce::HashMap<juce::ImagePixelData*, TextureCollection*> collectionByPixelData;

    juce::Array<TextureCollection*> disposedCollections;

    juce::Time currentTime = juce::Time::getCurrentTime();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CachedImages)
};

} // namespace

} // namespace parawave