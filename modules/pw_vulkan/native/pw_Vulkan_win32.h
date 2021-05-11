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

namespace juce
{
extern ComponentPeer* createNonRepaintingEmbeddedWindowsPeer (Component&, void* parent);

#if JUCE_WIN_PER_MONITOR_DPI_AWARE
 extern JUCE_API double getScaleFactorForWindow (HWND);
#endif

} // namespace juce

//==============================================================================

namespace parawave
{

//==============================================================================
/**
    Win32SurfaceKHR
*/
class Win32SurfaceKHR : public VulkanSurface
{
private:
    Win32SurfaceKHR() = delete;

public:
    Win32SurfaceKHR(const VulkanPhysicalDevice& physicalDevice, const vk::Win32SurfaceCreateInfoKHR& createInfo)
        : VulkanSurface(physicalDevice)
    {
        const auto& instance = physicalDevice.getInstance();

        vk::Result result;
                
        jassert(instance.getHandle());
        std::tie(result, handle) = instance.getHandle().createWin32SurfaceKHRUnique(createInfo).asTuple();

        PW_CHECK_VK_RESULT_SUCCESS(result, "Couldn't create Win32SurfaceKHR.");

        if (result == vk::Result::eSuccess && handle)
        {
            PW_DBG_V("Created Win32SurfaceKHR.");

            updateCapabilities();
        }
    }

    ~Win32SurfaceKHR() override
    {
        PW_DBG_V("Destroyed Win32SurfaceKHR.");
    }

    Win32SurfaceKHR(const VulkanPhysicalDevice& physicalDevice, HINSTANCE instanceHandle, HWND windowHandle)
        : Win32SurfaceKHR(physicalDevice, vk::Win32SurfaceCreateInfoKHR().setHinstance(instanceHandle).setHwnd(windowHandle)) {}

    static std::unique_ptr<VulkanSurface> create(const VulkanPhysicalDevice& physicalDevice, juce::Component& component, juce::ComponentPeer& componentPeer)
    {
        if (auto* topComp = component.getTopLevelComponent())
        {
            if (auto* peer = topComp->getPeer())
            {
                auto instanceHandle = (HINSTANCE)juce::Process::getCurrentModuleInstanceHandle();
                auto windowHandle = (HWND)componentPeer.getNativeHandle();

                return std::make_unique<Win32SurfaceKHR>(physicalDevice, instanceHandle, windowHandle);
            }
        }

        return {};
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Win32SurfaceKHR)
};

//==============================================================================
/**
    VulkanWin32Surface
*/
class VulkanWin32Surface final : public VulkanNativeSurface, private juce::Timer
{
private:
    struct DummyComponent  : public juce::Component
    {
        DummyComponent (VulkanWin32Surface& surface_) : surface(surface_) {}

        // The windowing code will call this when a paint callback happens
        void handleCommandMessage(int) override   
        { 
            /** This will invalidate the CachedImage in the VulkanContext. */
            surface.renderSurface();
        }

        VulkanWin32Surface& surface;
    };

public:
    VulkanWin32Surface (Target& surfaceTarget) : VulkanNativeSurface(surfaceTarget)
    {
        auto& component = target.getSurfaceComponent();
        
        dummyComponent.reset(new DummyComponent(*this));
        createNativeWindow(component);

        const auto refreshRate = surfaceTarget.getRefreshRate();
        startTimer(static_cast<int>(refreshRate));

        // Immediately update the window position to retrigger render and avoid lag of surface resize
        updateSurface();

        component.getTopLevelComponent()->repaint();
        component.repaint();
    }

    ~VulkanWin32Surface() override 
    {
        stopTimer();
    }

    juce::Rectangle<int> getBounds() const noexcept
    {
        return safeComponent != nullptr ? safeComponent->getBounds() : juce::Rectangle<int>();
    }

    //==============================================================================
    // implements : VulkanNativeSurface

    std::unique_ptr<VulkanSurface> createSurface(const VulkanPhysicalDevice& physicalDevice) const noexcept override
    {
        if (safeComponent != nullptr && nativeWindow.get() != nullptr)
            return Win32SurfaceKHR::create(physicalDevice, *safeComponent, *nativeWindow);
        
        return {};
    }

    vk::Extent2D getSurfaceExtent() const noexcept override
    {
        const auto bounds = getBounds();
        return vk::Extent2D(static_cast<uint32_t>(bounds.getWidth()), static_cast<uint32_t>(bounds.getHeight()));
    }

    double getSurfaceScale() const noexcept override
    {
       #if JUCE_WINDOWS && JUCE_WIN_PER_MONITOR_DPI_AWARE
        if (nativeWindow != nullptr)
            return juce::getScaleFactorForWindow ((HWND) nativeWindow->getNativeHandle());
       #endif

        return 1.0;
    }

    void updateSurfacePosition(juce::Rectangle<int> bounds) const noexcept override
    {
        updateWindowPosition(bounds);
    }

    /** Mostly called by the invalidation of the CachedComponentImage of the VulkanContext.
        Or a continuously running Timer. Because an immediate redraw would slow or freeze the window,
        only update the surface size and trigger a repaint. */
    void invalidateSurface() noexcept override
    {
        updateSurface();
        invalidateWindow();
    }

    //==============================================================================

    void renderSurface() noexcept
    {
        if (safeComponent != nullptr)
        {
            /** When resizing a window the native hwnd will immediately trigger a repaint.
                Since the surface bounds are not updated yet, the new area would be empty.
                Always update the surface bounds before rendering a new frame! */
            updateSurface();
            
            target.renderFrame();
        }
    }

private:
    void createNativeWindow(juce::Component& component)
    {
        auto* topComp = component.getTopLevelComponent();

        {
            auto* parentHWND = topComp->getWindowHandle();

            juce::ScopedThreadDPIAwarenessSetter setter { parentHWND };
            nativeWindow.reset (createNonRepaintingEmbeddedWindowsPeer (*dummyComponent, parentHWND));
        }

        if (auto* peer = topComp->getPeer())
        {
            safeComponent = juce::Component::SafePointer<juce::Component>(&component);

            updateWindowPosition(peer->getAreaCoveredBy (component));
        }

        nativeWindow->setVisible (true);
    }

    /** For smooth resize and recreation of the swap chain, the window position must be updated ! */
    void updateSurface()
    {
        if (safeComponent != nullptr)
        {
            if (auto* peer = safeComponent->getTopLevelComponent()->getPeer())
            {
                auto newScale = peer->getPlatformScaleFactor();
                nativeScaleFactor = newScale;

                updateWindowPosition(peer->getAreaCoveredBy (*safeComponent));
            }
        }
    }

    void updateWindowPosition(juce::Rectangle<int> bounds) const noexcept
    {
        if (nativeWindow)
        {
            if (!juce::approximatelyEqual(nativeScaleFactor, 1.0))
                bounds = (bounds.toDouble() * nativeScaleFactor).toNearestInt();

            SetWindowPos ((HWND) nativeWindow->getNativeHandle(), nullptr,
                          bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                          SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
        }
    }
    
    /** Triggers a WM_PAINT. The paint is not immediate, so it doesn't interfere with events triggered by window resizing. */ 
    void invalidateWindow()
    {
        if (nativeWindow)
            RedrawWindow((HWND)nativeWindow->getNativeHandle(), nullptr, nullptr, RDW_INVALIDATE);
    }

    void timerCallback() override
    {
        invalidateWindow();
    }

private:
    std::unique_ptr<DummyComponent> dummyComponent;
    std::unique_ptr<juce::ComponentPeer> nativeWindow;

    juce::Component::SafePointer<juce::Component> safeComponent;

    double nativeScaleFactor = 1.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanWin32Surface)
};

//==============================================================================
// implementation : pw_VulkanNativeSurface

std::unique_ptr<VulkanNativeSurface> VulkanNativeSurface::create(VulkanNativeSurface::Target& surfaceTarget)
{
    return std::make_unique<VulkanWin32Surface>(surfaceTarget);
}

} // namespace parawave