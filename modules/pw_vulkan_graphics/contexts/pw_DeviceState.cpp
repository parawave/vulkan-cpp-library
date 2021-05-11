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
class DeviceState
{
private:
    struct Cache
    {
        Cache(VulkanDevice& device_, vk::Format renderFormat) : device(device_)
        {
            memory = CachedMemory::get(device);
            shaders = CachedShaders::get(device);
            images = CachedImages::get(device, *memory);
            renderPasses = CachedRenderPasses::get(device, renderFormat);
            pipelines = CachedPipelines::get(device, *images, *renderPasses);
        }

        VulkanDevice& device;

        CachedMemory::Ptr memory;
        CachedShaders::Ptr shaders;
        CachedImages::Ptr images;
        CachedRenderPasses::Ptr renderPasses;
        CachedPipelines::Ptr pipelines;
    };

public:
    DeviceState(VulkanDevice& device_, vk::Format renderFormat = vk::Format::eUndefined) :
        cache(device_, renderFormat),
        device(device_),
        memory(*cache.memory), shaders(*cache.shaders), images(*cache.images), 
        renderPasses(*cache.renderPasses), pipelines(*cache.pipelines) 
    { }

    virtual ~DeviceState()
    {
        if(minimizeOnRelease)
            minimizeStorage();
    }

    bool useMinimizeStorageOnRelease() const noexcept { return minimizeOnRelease; }

    void setMinimizeStorageOnRelease(bool newState)
    {
        minimizeOnRelease = newState;
    }

    VulkanDevice& getDevice() const noexcept { return cache.device; }

    void minimizeStorage(bool forceMinimize = false)
    {
        // If a new Device State is created or destroyed, it's a good chance to minimize
        // the memory used in previous allocations, e.g. the framebuffer storage.

        memory.minimizeStorage(forceMinimize);
    }

private:
    Cache cache;

public:
    const VulkanDevice& device;

    CachedMemory& memory;
    CachedShaders& shaders;
    CachedImages& images;
   
    const CachedRenderPasses& renderPasses;
    const CachedPipelines& pipelines;

private:
    bool minimizeOnRelease = true;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeviceState)
};

} // namespace parawave
