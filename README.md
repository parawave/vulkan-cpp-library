**Parawave Vulkan C++** is a collection of [JUCE](https://github.com/juce-framework/JUCE) modules that extend the application framework by adding the [LunarG Vulkan SDK](https://www.lunarg.com/vulkan-sdk/).

The provided modules can be used together with the regular [juce_graphics](https://docs.juce.com/master/group__juce__graphics.html) and [juce_gui_basics](https://docs.juce.com/master/group__juce__gui__basics.html) modules to paint hardware accelerated graphics to a native surface.

------

## Important

At the current time the modules are developed and tested on Windows 10 only. Although possible, there is no support for MacOS / iOS / Linux or Android yet. Hopefully this will change. Any help and contribution is appreciated.

------

## Getting Started

### Requirements

- [JUCE 6](https://github.com/juce-framework/JUCE)
- [Vulkan SDK 1.2.170.0](https://vulkan.lunarg.com/sdk/home)

The code is intended to be used together with the JUCE framework, therefore the same general [requirements](https://github.com/juce-framework/JUCE#minimum-system-requirements) apply.

### Projucer

Add the [pw_vulkan]() and [pw_vulkan_graphics]() modules to your Projucer project.

The modules provided here depend on header and library files of the Vulkan SDK, therefore it's necessary to manually add the include and library directories to your Projucer project. e.g. for version 1.2.170.0 of the SDK, the paths are:

- **Header Search Paths** : C:\VulkanSDK\1.2.170.0\Include\
- **Extra Library Search Paths** : C:\VulkanSDK\1.2.170.0\Lib

------

## Examples

### Attaching a Context

Similar to the [juce::OpenGLContext](https://docs.juce.com/master/classOpenGLContext.html) a [parawave::VulkanContext]() can be attached to a [juce::Component](https://docs.juce.com/master/classComponent.html) to activate it for use with [juce::Graphics](https://docs.juce.com/master/classGraphics.html). Alternatively your component could directly extend from [parawave::VulkanAppComponent](), similar to [juce::OpenGLAppComponent](https://docs.juce.com/master/classOpenGLAppComponent.html).

```c++
class MainComponent : public juce::Component
{
public:
    MainComponent()
    {
        context.setDefaultPhysicalDevice(instance);
        context.attachTo(*this);
    }
    
    ~MainComponent() override
    {
        context.detach();
    }
    
    void paint (juce::Graphics& g) override
    {
        // Draw with Vulkan ...
    }

    parawave::VulkanInstance instance;
    parawave::VulkanContext context;
};
```

------

## Modules

### pw_vulkan

This module provides wrapper classes around all major Vulkan vk::Unique... structures to simplify the setup process of Vulkan objects. **VulkanDevice** for example holds a **vk::UniqueDevice** (vkDevice) and simplifies the device creation by hiding the device extension selection in the implementation.

There are also a few utility classes for common tasks like memory allocation, image/buffer transfers and fence/semaphore synchronization.

### pw_vulkan_graphics

This module depends on pw_vulkan and implements a [juce::LowLevelGraphicsContext](https://docs.juce.com/master/classLowLevelGraphicsContext.html) in **VulkanContext** similar to [juce::OpenGLContext](https://docs.juce.com/master/classOpenGLContext.html). A **VulkanImageType** also allows the use of regular [juce::Image](https://docs.juce.com/master/classImage.html) objects as render target for [juce::Graphics](https://docs.juce.com/master/classGraphics.html) operations.

------

## Contribution / What to do!?

How can you contribute and what steps are necessary to get to a production ready state?

- Add the native surface implementation for MacOS / iOS / Linux and Android.
- Try to run [MoltenVK](https://github.com/KhronosGroup/MoltenVK) on MacOS and iOS and simplify the SDK setup process together with the Projucer.
- Add test cases for common scenarios like swapchain recreation and the frequent recreation of resources.
- Add documentation.
- Add examples for the custom setup of a full render pipeline.
- Add customization to the graphics context for user defined shaders, graphic or compute pipelines.
- Add compute shaders for hardware accelerated image pixel processing.
- Improve the performance of the scanline rasterizer implementation of the juce::LowLevelGraphicsContext.

------

## References

- **Vulkan SDK** : https://www.lunarg.com/vulkan-sdk/
- **Vulkan SDK - Getting Started :** https://vulkan.lunarg.com/doc/sdk/1.2.170.0/windows/getting_started.html
- **Vulkan-Hpp** : https://github.com/KhronosGroup/Vulkan-Hpp
- **Vulkan Specification** : https://www.khronos.org/registry/vulkan/specs/1.2/html/index.html
- **Vulkan Hardware Database** : http://vulkan.gpuinfo.org/
- **Vulkan Tutorial** : https://vulkan-tutorial.com/
- **Vulkan C++ Examples** : https://github.com/SaschaWillems/Vulkan
