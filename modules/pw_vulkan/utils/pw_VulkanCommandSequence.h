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
    VulkanCommandSequence

    Helper class to submit commands (and chain command buffers) with semaphores 
    and optional fence. 
*/
class VulkanCommandSequence
{
public:
    struct SingleSubmit : public vk::SubmitInfo
    {
        SingleSubmit(const VulkanCommandBuffer& commandBuffer)
        {
            buffers[0] = commandBuffer.getHandle();

            setCommandBuffers(buffers);
        }

        std::array<vk::CommandBuffer, 1> buffers;
    };

    //==============================================================================
    struct SingleWaitSignalSubmit : public SingleSubmit
    {
        SingleWaitSignalSubmit(const VulkanCommandBuffer& commandBuffer)
            : SingleSubmit(commandBuffer) {}

        SingleWaitSignalSubmit& setWaitSemaphore(const VulkanSemaphore& waitSemaphore) noexcept
        {
            waits[0] = waitSemaphore.getHandle();
            setWaitSemaphores(waits);

            setWaitDstStageMask(waitStages);

            return *this;
        }

        SingleWaitSignalSubmit& setSignalSemaphore(const VulkanSemaphore& signalSemaphore) noexcept
        {
            signals[0] = signalSemaphore.getHandle();
            setSignalSemaphores(signals);

            return *this;
        }

        std::array<vk::Semaphore, 1> waits;
        std::array<vk::Semaphore, 1> signals;

        std::array<vk::PipelineStageFlags, 1> waitStages = 
        { 
            vk::PipelineStageFlagBits::eColorAttachmentOutput 
        };
    };

    //==============================================================================
private:
    VulkanCommandSequence() = delete;

public:
    explicit VulkanCommandSequence(const VulkanDevice& device_) :
        device(device_), completedFence(device) {}

    virtual ~VulkanCommandSequence() = default;

    const VulkanSemaphore* getCurrentWaitSemaphore() const noexcept { return currentWaitSemaphore; }

    void setCurrentWaitSemaphore(VulkanSemaphore* newSemaphore) noexcept { currentWaitSemaphore = newSemaphore; }

    const VulkanFence& getCompletedFence() const noexcept { return completedFence; }

    template<typename CommandsFunction>
    void submit(const CommandsFunction& commandsFunction, bool useFence = false)
    {
        auto commandBuffer = commandBuffers.add(new VulkanCommandBuffer(device));

        commandBuffer->begin();
         commandsFunction(*commandBuffer);
        commandBuffer->end();

        auto signalSemaphore = semaphores.add(new VulkanSemaphore(device));

        submit(*commandBuffer, currentWaitSemaphore, signalSemaphore, useFence);

        // Current signal semaphore is the next wait semaphore !
        currentWaitSemaphore = signalSemaphore;
    }

    void waitForFence(juce::RelativeTime duration = juce::RelativeTime::milliseconds(10)) noexcept
    {
        if (! fenceInUseFlag)
            return;
            
        const auto& fence = getCompletedFence();
        fence.waitIdle(duration);

        fenceInUseFlag = false;
    }

private:
    void submit(const VulkanCommandBuffer& commandBuffer, const VulkanSemaphore* waitSemaphore, const VulkanSemaphore* signalSemaphore, bool useFence)
    {
        const auto& queue = device.getGraphicsQueue();

        auto submitInfo = SingleWaitSignalSubmit(commandBuffer);

        if (waitSemaphore)
            submitInfo.setWaitSemaphore(*waitSemaphore);

        if (signalSemaphore)
            submitInfo.setSignalSemaphore(*signalSemaphore);

        if (useFence)
        {
            waitForFence();
            completedFence.reset();
        }
          
        queue.submit(submitInfo, useFence ? completedFence.getHandle() : nullptr);

        if (useFence)
            fenceInUseFlag = true;
    }

protected:
    const VulkanDevice& device;

private:
    juce::OwnedArray<VulkanCommandBuffer> commandBuffers;
    juce::OwnedArray<VulkanSemaphore> semaphores;
   
    const VulkanSemaphore* currentWaitSemaphore = nullptr;

    const VulkanFence completedFence;

    bool fenceInUseFlag = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VulkanCommandSequence)
};
    
} // namespace parawave