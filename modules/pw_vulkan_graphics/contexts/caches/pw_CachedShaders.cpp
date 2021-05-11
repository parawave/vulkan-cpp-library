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
struct CachedShaders : public juce::ReferenceCountedObject
{
public:
    using Ptr = juce::ReferenceCountedObjectPtr<CachedShaders>;

    CachedShaders() = delete;

    explicit CachedShaders(const VulkanDevice& d) : device(d)
    {
        loadModule("Basic.vert", vertBasic, vertBasicSize);
        loadModule("Image.vert", vertImage, vertImageSize);
        loadModule("Image.frag", fragImage, fragImageSize);
        loadModule("LinearGradient.vert", vertLinearGradient, vertLinearGradientSize);
        loadModule("LinearGradient1.frag", fragLinearGradient1, fragLinearGradient1Size);
        loadModule("LinearGradient2.frag", fragLinearGradient2, fragLinearGradient2Size);
        loadModule("Overlay.vert", vertOverlay, vertOverlaySize);
        loadModule("Overlay.frag", fragOverlay, fragOverlaySize);
        loadModule("RadialGradient.vert", vertRadialGradient, vertRadialGradientSize);
        loadModule("RadialGradient.frag", fragRadialGradient, fragRadialGradientSize);
        loadModule("SolidColour.vert", vertSolidColour, vertSolidColourSize);
        loadModule("SolidColour.frag", fragSolidColour, fragSolidColourSize);
        loadModule("TiledImage.frag", fragTiledImage, fragTiledImageSize);
        loadModule("TiledImage.vert", vertTiledImage, vertTiledImageSize);
    }

    static CachedShaders* get(VulkanDevice& device)
    {
        static constexpr char objectID[] = "CachedShaders";
        auto shaders = static_cast<CachedShaders*>(device.getAssociatedObject (objectID));
        if (shaders == nullptr)
        {
            shaders = new CachedShaders(device);
            device.setAssociatedObject(objectID, shaders);
        }

        return shaders;
    }

    void loadModule(const char* name, const void* spvData, const int dataSize)
    {
        const auto data = reinterpret_cast<const uint32_t*>(spvData);
        const auto size = static_cast<size_t>(dataSize);

        auto shader = shaderModules.add(new VulkanShaderModule(device, data, size));
        setShaderModule(name, shader);
    }

    const VulkanShaderModule* getShaderModule(const char* name) const
    {
        jassert (name != nullptr);

        auto index = shaderNames.indexOf (name);
        return index >= 0 ? shaders.getUnchecked(index) : nullptr;
    }

    void setShaderModule(const char* name, VulkanShaderModule* newObject)
    {
        jassert (name != nullptr);

        const int index = shaderNames.indexOf (name);

        if (index >= 0)
        {
            if (newObject != nullptr)
            {
                shaders.set (index, newObject);
            }
            else
            {
                shaderNames.remove (index);
                shaders.remove (index);
            }
        }
        else if (newObject != nullptr)
        {
            shaderNames.add (name);
            shaders.add (newObject);
        }
    }

private:
    const VulkanDevice& device;

    juce::StringArray shaderNames;
    juce::Array<VulkanShaderModule*> shaders;

    juce::OwnedArray<VulkanShaderModule> shaderModules;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CachedShaders)
};

} // namespace

} // namespace parawave