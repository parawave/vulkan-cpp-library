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

namespace RenderPass
{

struct OffscreenPassInfo : public vk::RenderPassCreateInfo
{
    OffscreenPassInfo(vk::Format format) : colourFormat(format)
    {
        auto& colorAttachment = attachments[0];
        auto& subpass = subpasses[0];

        colorAttachment
            .setFormat(colourFormat)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

        subpass
            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachments(colourAttachments);

        // TODO: We could potentially add some more advanced render pass dependencies.
        // Not sure if this is necessary for the general offscreen render case.

        /*
        dependencies[0]
            .setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setSrcStageMask(vk::PipelineStageFlagBits::eFragmentShader)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
            .setDependencyFlags(vk::DependencyFlagBits::eByRegion);

        dependencies[1]
            .setSrcSubpass(0)
            .setDstSubpass(VK_SUBPASS_EXTERNAL)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
            .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
            .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
            .setDependencyFlags(vk::DependencyFlagBits::eByRegion);
        */

        /*
        dependencies[0]
            .setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
            .setDependencyFlags(vk::DependencyFlagBits::eByRegion);
          
        dependencies[1]
            .setSrcSubpass(0)
            .setDstSubpass(VK_SUBPASS_EXTERNAL)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
            .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
            .setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
            .setDependencyFlags(vk::DependencyFlagBits::eByRegion);
        */

        setAttachments(attachments);
        setSubpasses(subpasses);
        setDependencies(dependencies);
    }

    vk::Format colourFormat;

    std::array<vk::AttachmentReference, 1> colourAttachments = 
    { 
        vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal) 
    };

    std::array<vk::AttachmentDescription, 1> attachments;
    std::array<vk::SubpassDescription, 1> subpasses;
    std::array<vk::SubpassDependency, 0> dependencies;
};

//==============================================================================
struct SwapchainPassInfo : public vk::RenderPassCreateInfo
{
    SwapchainPassInfo(vk::Format format) : colourFormat(format)
    {
        auto& colorAttachment = attachments[0];
        auto& subpass = subpasses[0];

        colorAttachment
            .setFormat(colourFormat)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

        subpass
            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachments(colourAttachments);

        setAttachments(attachments);
        setSubpasses(subpasses);
        setDependencies(dependencies);
    }

    vk::Format colourFormat;

    std::array<vk::AttachmentReference, 1> colourAttachments = 
    { 
        vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal) 
    };

    std::array<vk::AttachmentDescription, 1> attachments;
    std::array<vk::SubpassDescription, 1> subpasses;
    std::array<vk::SubpassDependency, 0> dependencies;
};

} // namespace RenderPass

namespace
{

//==============================================================================
struct CachedRenderPasses : public juce::ReferenceCountedObject
{
    using Ptr = juce::ReferenceCountedObjectPtr<CachedRenderPasses>;
    
    CachedRenderPasses(const VulkanDevice& device, vk::Format format) : 
        offscreen(device, RenderPass::OffscreenPassInfo(format)),
        swapchain(device, RenderPass::SwapchainPassInfo(format)) {}

    static CachedRenderPasses* get(VulkanDevice& device, vk::Format format)
    {
        static constexpr char objectID[] = "CachedRenderPasses";
        auto renderPasses = static_cast<CachedRenderPasses*>(device.getAssociatedObject (objectID));
        if (renderPasses == nullptr)
        {
            /** An undefined format is not allowed!
                The first time the render passes are created, a valid swapchain format should be passed. */
            jassert(format != vk::Format::eUndefined);

            renderPasses = new CachedRenderPasses(device, format);
            device.setAssociatedObject(objectID, renderPasses);
        }

        return renderPasses;
    }

    VulkanRenderPass offscreen;
    VulkanRenderPass swapchain;
};

} // namespace

} // namespace parawave