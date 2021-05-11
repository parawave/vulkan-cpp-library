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
class VulkanContext::CachedImage  : public juce::CachedComponentImage, public VulkanNativeSurface::Target
{
public:
    CachedImage (VulkanContext& c, juce::Component& comp)
        : context (c), component (comp)
    {  
        nativeSurface = VulkanNativeSurface::create(*this);
        surface = nativeSurface->createSurface(context.device->getPhysicalDevice());

        createSwapchain();  
    }

    ~CachedImage() override
    {
        if (const auto cd = context.device.get())
            cd->waitIdle();
    }

    void paint(juce::Graphics& ) override { }

    bool invalidateAll() override
    {
        validArea.clear();
        
        triggerRepaint();
        return false;
    }

    bool invalidate(const juce::Rectangle<int>& area) override
    {
        const auto transform = getPaintTransform();
        validArea.subtract (area.toFloat().transformedBy(transform).getSmallestIntegerContainer());
        
        triggerRepaint();
        return false;
    }

    void releaseResources() override { }

    static CachedImage* get(juce::Component& c) noexcept
    {
        return dynamic_cast<CachedImage*> (c.getCachedComponentImage());
    }

    //==============================================================================

    bool isFullscreen() const noexcept
    {
        if (auto topLevelComponent = component.getTopLevelComponent())
        {
            const auto kioskModeComponent = juce::Desktop::getInstance().getKioskModeComponent();
            return topLevelComponent == kioskModeComponent;
        }

        return false;
    }

    void setFullscreen(bool useFullscreen)
    {
        if (useFullscreen != isFullscreen())
        {
            fullscreenFlag = useFullscreen;
            needsFullscreenChange = true;   
        }
    }

    void handleResize()
    {
        if (auto* peer = component.getTopLevelComponent()->getPeer())
        {
            if (nativeSurface)
                nativeSurface->updateSurfacePosition(peer->getAreaCoveredBy(component));
        }
    }

    void triggerRepaint()
    {
        if(nativeSurface)
            nativeSurface->invalidateSurface();
    }

    double getRenderingScale() const noexcept
    {
        return nativeSurface ? nativeSurface->getSurfaceScale() : 1.0;
    }

    bool isRendering() const noexcept { return renderingFlag; }

    //==============================================================================
    // implements : VulkanNativeSurface::Target
    
    juce::Component& getSurfaceComponent() override { return component; }

    void renderFrame() override
    {
        renderingFlag = true;

        if (auto r = renderContext.get())
        {
            auto status = r->drawFrame([&](RenderContext::FrameType& frame)
            {
                paintComponent(frame);
            });

            switch (status)
            {
                case RenderContext::DrawStatus::needsSwapChainRecreation:
                    needsSwapchainRecreation = true;
                    break;
                case RenderContext::DrawStatus::hasFailed:
                    //jassertfalse;
                    break;
                default:
                    break;
            }
        }

        checkSwapchainRecreation();

        renderingFlag = false;

        checkFullscreenChange();
    }

    /** In milliseconds */
    virtual uint32_t getRefreshRate() const override { return 9; } // 9ms about 100 fps on a non v-sync display due to inaccuracy?

    //==============================================================================

    juce::AffineTransform getPaintTransform() const
    {
        if (auto* peer = component.getPeer())
        {
            const auto localBounds = component.getLocalBounds();
            const auto displayScale = juce::Desktop::getInstance().getDisplays().getDisplayForRect (component.getTopLevelComponent()->getScreenBounds())->scale;

            const auto newArea = peer->getComponent().getLocalArea (&component, localBounds).withZeroOrigin().toDouble() * displayScale;

            return juce::AffineTransform::scale ((float) newArea.getWidth()  / (float) localBounds.getWidth(),
                                           (float) newArea.getHeight() / (float) localBounds.getHeight());
        }

        return juce::AffineTransform();
    }

    void paintComponent(RenderContext::FrameType& frame)
    {
        // you mustn't set your own cached image object when attaching a context!
        jassert(get (component) == this);

        const auto viewportArea = frame.getBounds();

        juce::RectangleList<int> invalid(viewportArea);
        invalid.subtract(validArea);
        validArea = viewportArea;

        if (!invalid.isEmpty())
        {
            std::unique_ptr<juce::LowLevelGraphicsContext> g(createVulkanGraphicsContext(frame));

            g->clipToRectangleList(invalid);

            const auto transform = getPaintTransform();
            g->addTransform(transform);

            paintOwner(*g);
        }
    }

    void paintOwner(juce::LowLevelGraphicsContext& llgc)
    {
        juce::Graphics g(llgc);

      #if JUCE_ENABLE_REPAINT_DEBUGGING
       #ifdef JUCE_IS_REPAINT_DEBUGGING_ACTIVE
        if (JUCE_IS_REPAINT_DEBUGGING_ACTIVE)
       #endif
        {
            g.saveState();
        }
      #endif

        JUCE_TRY
        {
            component.paintEntireComponent(g, false);
        }
        JUCE_CATCH_EXCEPTION

      #if JUCE_ENABLE_REPAINT_DEBUGGING
       #ifdef JUCE_IS_REPAINT_DEBUGGING_ACTIVE
        if (JUCE_IS_REPAINT_DEBUGGING_ACTIVE)
       #endif
        {
            // enabling this code will fill all areas that get repainted with a colour overlay, to show
            // clearly when things are being repainted.
            g.restoreState();

            static juce::Random rng;
            g.fillAll(juce::Colour ((uint8_t) rng.nextInt (255),
                                    (uint8_t) rng.nextInt (255),
                                    (uint8_t) rng.nextInt (255),
                                    (uint8_t) 0x50));
        }
       #endif
    }

public:
    VulkanContext& context;

private:
    VulkanSwapchain::CreateInfo getSwapchainCreateInfo(const VulkanSwapchain* oldSwapchain = nullptr)
    {
        jassert(context.device && surface);
        return VulkanSwapchain::CreateInfo(*context.device, *surface, context.format, context.colorSpace, context.presentMode, oldSwapchain);
    }

    void createSwapchain()
    {
        if (const auto cd = context.device.get())
        {
            const auto createInfo = getSwapchainCreateInfo();

            swapchain.reset(new VulkanSwapchain(*cd, *surface, createInfo));
            renderContext.reset(new RenderContext(*cd, *swapchain));
        }
    }

    void recreateSwapchain()
    {
        renderContext.reset();
        
        /** The old swap chain will be passed to the create info of the new swap chain. */
        std::unique_ptr<VulkanSwapchain> oldSwapchain;
        oldSwapchain.swap(swapchain);
        
        if (const auto cd = context.device.get())
        {
            // Before the swap chain is recreated, get the newest surface capabilities !
            surface->updateCapabilities();

            const auto createInfo = getSwapchainCreateInfo(oldSwapchain.get());
            if (createInfo.isValid())
            {
                swapchain.reset(new VulkanSwapchain(*cd, *surface, createInfo));
                renderContext.reset(new RenderContext(*cd, *swapchain));

                validArea.clear();

                needsSwapchainRecreation = false;
            }
        }
    }

    void checkSwapchainRecreation()
    {
        if (needsSwapchainRecreation)
        {
            recreateSwapchain();
        }
    }

    void checkFullscreenChange()
    {
        if (!needsFullscreenChange)
            return;

        if (auto topLevelComponent = component.getTopLevelComponent())
        {
            // Can only switch to fullscreen if the component is visible on desktop
            if (juce::ComponentPeer::getPeerFor(topLevelComponent) == nullptr)
                return;

            auto& desktop = juce::Desktop::getInstance();

            const auto isKioskComponent = desktop.getKioskModeComponent() == topLevelComponent;

            if (fullscreenFlag && !isKioskComponent)
                desktop.setKioskModeComponent(topLevelComponent, false);
            else if(!fullscreenFlag && isKioskComponent)
                desktop.setKioskModeComponent(nullptr, false);

            needsFullscreenChange = false;
        }
    }

private:
    juce::Component& component;

    std::unique_ptr<VulkanNativeSurface> nativeSurface;
    std::unique_ptr<VulkanSurface> surface;

    std::unique_ptr<VulkanSwapchain> swapchain;
    std::unique_ptr<RenderContext> renderContext;

    juce::RectangleList<int> validArea;

    bool needsSwapchainRecreation = false;
    bool needsFullscreenChange = false;
    
    bool renderingFlag = false;
    bool fullscreenFlag = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CachedImage)
};

//==============================================================================
class VulkanContext::Attachment : public juce::ComponentMovementWatcher
{
public:
    Attachment (VulkanContext& c, juce::Component& comp)
       : juce::ComponentMovementWatcher (&comp), context (c)
    {
        if (canBeAttached (comp))
            attachCachedComponentImage();
    }

    ~Attachment() override
    {
        detachCachedComponentImage();
    }

    /** Clear the ReferenceCounted cached objects.*/
    static void clearCachedObjects(VulkanDevice& device)
    {
        // Only perform cached object clear if there are object. Avoid unnecessary recreation!
        if (device.hasAssociatedObject())
        {
            // We get the reference counted pointers to achieve the deletion in the correct order.
            // e.g. An allocated VulkanMemoryImage in 'CachedImages' uses a VulkanMemoryPool in 'CachedMemory'
            // so we shouldn't delete the CachedMemory before CachedImages is freed !

            CachedMemory::Ptr memory = CachedMemory::get(device);
            CachedShaders::Ptr shaders = CachedShaders::get(device);
            CachedImages::Ptr images = CachedImages::get(device, *memory);
            CachedRenderPasses::Ptr renderPasses = CachedRenderPasses::get(device, vk::Format::eUndefined);
            CachedPipelines::Ptr pipelines = CachedPipelines::get(device, *images, *renderPasses);
            
            // First clear the remaining cache object not referenced in the current scope
            device.clearAssociatedObjects();

            // Clear the remaining reference when leaving the block scope !
        }
    }

    void minimizeStorage()
    {
        if (auto cd = context.getDevice())
        {
            CachedMemory::Ptr cachedMemory = CachedMemory::get(*cd);
            
            cachedMemory->minimizeStorage(true);

            #if(JUCE_DEBUG == 1)
            cachedMemory->printUsage();
            #endif
        } 
    }

    void detachCachedComponentImage()
    {
        auto& comp = *getComponent();

        if (isAttached(comp))
        {
            if (auto* c = CachedImage::get(comp))
            {
                if (c->isRendering())
                {
                    /** 
                        Do not trigger the recreation of the context during rendering!
                        @see VulkanContext::setFullscreen();
                    */
                    jassertfalse;
                    return;
                }
            }

            comp.setCachedComponentImage(nullptr);
            PW_DBG_V("Detached Vulkan context.");

            // If the cached image is deleted, it's probably due to window minimization.
            // We can force the deallocation of unneeded resources (like framebuffer storage).
            minimizeStorage();
        }
    }

    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/) override
    {
        auto& comp = *getComponent();
        
        if (isAttached(comp) != canBeAttached(comp))
            componentVisibilityChanged();
  
        if (comp.getWidth() > 0 && comp.getHeight() > 0)
        {
            if (auto* c = CachedImage::get(comp))
                c->handleResize();
        }
    }

    using ComponentMovementWatcher::componentMovedOrResized;

    void componentPeerChanged() override
    {
        detachCachedComponentImage();
        componentVisibilityChanged();
    }

    void componentVisibilityChanged() override
    {
        auto& comp = *getComponent();

        if (canBeAttached (comp))
        {
            if (isAttached(comp))
                comp.repaint(); // (needed when windows are un-minimised)
            else
                attachCachedComponentImage();
        }
        else
        {
            detachCachedComponentImage();
        }
    }

    using ComponentMovementWatcher::componentVisibilityChanged;

   #if JUCE_DEBUG || JUCE_LOG_ASSERTIONS
    void componentBeingDeleted (juce::Component& c) override
    {
        /* You must call detach() or delete your VulkanContext to remove it
           from a component BEFORE deleting the component that it is using!
        */
        jassertfalse;

        ComponentMovementWatcher::componentBeingDeleted (c);
    }
   #endif

private:
    VulkanContext& context;

    bool canBeAttached(const juce::Component& comp) noexcept
    {
        bool validPeerBounds = false;
        
        if (auto* topComp = comp.getTopLevelComponent())
        {
            if (auto* peer = topComp->getPeer())
            {
                const auto bounds = peer->getBounds();
                validPeerBounds = bounds.getWidth() > 0 && bounds.getHeight() > 0;
            }
        }
        
        return validPeerBounds && comp.getWidth() > 0 && comp.getHeight() > 0 && isShowingOrMinimised(comp);
    }

    static bool isShowingOrMinimised (const juce::Component& c)
    {
        if (! c.isVisible())
            return false;

        if (auto* p = c.getParentComponent())
            return isShowingOrMinimised (*p);

        return c.getPeer() != nullptr;
    }

    static bool isAttached (const juce::Component& comp) noexcept
    {
        return comp.getCachedComponentImage() != nullptr;
    }

    void attachCachedComponentImage()
    {
        auto& comp = *getComponent();
     
        // Only attach if the cached image was correctly deleted !
        if (comp.getCachedComponentImage())
            return;

        auto* newCachedImage = new CachedImage (context, comp);
        
        comp.setCachedComponentImage (newCachedImage);
        PW_DBG_V("Attached Vulkan context.");

        comp.repaint();
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Attachment)
};

//==============================================================================
VulkanContext::VulkanContext() = default;

VulkanContext::~VulkanContext()
{
    resetPhysicalDevice();
}

bool VulkanContext::isFullscreen() const noexcept
{
    if (auto* cachedImage = getCachedImage())
        return cachedImage->isFullscreen();

    return false;
}

void VulkanContext::setFullscreen(bool useFullscreen)
{
    if (auto* cachedImage = getCachedImage())
        cachedImage->setFullscreen(useFullscreen);
}

void VulkanContext::setFormat(vk::Format preferredFormat)
{
    // This method must not be called when the context has already been attached!
    // Call it before attaching your context, or use detach() first, before calling this!
    jassert(! attachment);

    format = preferredFormat;
}

void VulkanContext::setColourSpace(vk::ColorSpaceKHR preferredColorSpace)
{
    // This method must not be called when the context has already been attached!
    // Call it before attaching your context, or use detach() first, before calling this!
    jassert(! attachment);

    colorSpace = preferredColorSpace;
}

void VulkanContext::setPresentMode(vk::PresentModeKHR preferredPresentMode)
{
    // This method must not be called when the context has already been attached!
    // Call it before attaching your context, or use detach() first, before calling this!
    jassert(! attachment);

    presentMode = preferredPresentMode;
}

void VulkanContext::setPhysicalDevice(const VulkanPhysicalDevice& physicalDevice)
{
    // This method must not be called when the context has already been attached!
    // Call it before attaching your context, or use detach() first, before calling this!
    jassert (! attachment.get());

    if (device.get() != nullptr)
        return;

    resetPhysicalDevice();

    device.reset(new VulkanDevice(physicalDevice));
}

void VulkanContext::setDefaultPhysicalDevice(const VulkanInstance& instance)
{
    if (auto defaultDevice = instance.getPhysicalDevices().getFirst())
        setPhysicalDevice(*defaultDevice);
    else
    {
        PW_DBG_V("Couldn't find default device! Are Vulkan drivers installed on the system?");
        jassertfalse;

        resetPhysicalDevice();
    }
}

void VulkanContext::resetPhysicalDevice()
{
    if (device.get() != nullptr)
        Attachment::clearCachedObjects(*device);

    detach();
    device.reset();
}

void VulkanContext::attachTo(juce::Component& component)
{
    // Set the physical device before attaching a component to your context!
    jassert (device.get());

    if (device.get() == nullptr)
        return;

    component.repaint();

    if (getTargetComponent() != &component)
    {
        detach();
        attachment.reset (new Attachment (*this, component));
    }
}

void VulkanContext::detach()
{
    if (auto* a = attachment.get())
    {
        a->detachCachedComponentImage(); // must detach before nulling our pointer
        attachment.reset();
    }
}

bool VulkanContext::isAttached() const noexcept
{
    return attachment != nullptr;
}

VulkanDevice* VulkanContext::getDevice() const noexcept { return device.get(); }

double VulkanContext::getRenderingScale() const noexcept
{
    if (auto* cachedImage = getCachedImage())
        return cachedImage->getRenderingScale();

    return 1.0;
}

juce::Component* VulkanContext::getTargetComponent() const noexcept
{
    return attachment != nullptr ? attachment->getComponent() : nullptr;
}

VulkanContext* VulkanContext::getContextAttachedTo(juce::Component& c) noexcept
{
    if (auto* ci = CachedImage::get (c))
        return &(ci->context);

    return nullptr;
}

void VulkanContext::triggerRepaint()
{
    if (auto* cachedImage = getCachedImage())
        cachedImage->triggerRepaint();
}

VulkanContext::CachedImage* VulkanContext::getCachedImage() const noexcept
{
    if (auto* comp = getTargetComponent())
        return CachedImage::get (*comp);

    return nullptr;
}

} // namespace parawave