#include "vulkan/Debug.h"
#include "vulkan/handles/Device.h"
#include "Queue.h"
#include "Fence.h"
#include "CommandBuffer.h"
#include "Semaphore.h"

namespace Utopian::Vk
{
   Queue::Queue(Device* device)
      : Handle(device, nullptr)
   {
      // Get the queue from the device
      // [NOTE] that queueFamilyIndex is hard coded to 0
      uint32_t queueFamilyIndex = device->GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
      vkGetDeviceQueue(GetVkDevice(), queueFamilyIndex, 0, &mHandle);
   }

   Queue::~Queue()
   {
   }

   void Queue::Submit(CommandBuffer* commandBuffer, Fence* renderFence, const SharedPtr<Semaphore>& waitSemaphore, const SharedPtr<Semaphore>& signalSemaphore)
   {
      VkSubmitInfo submitInfo = {};

      //VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
      VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

      if (waitSemaphore != nullptr)
      {
         submitInfo.waitSemaphoreCount = 1;
         submitInfo.pWaitSemaphores = waitSemaphore->GetVkHandlePtr();
      }
      else
         submitInfo.waitSemaphoreCount = 0;

      if (signalSemaphore != nullptr)
      {
         submitInfo.signalSemaphoreCount = 1;
         submitInfo.pSignalSemaphores = signalSemaphore->GetVkHandlePtr();    // swapChain.queuePresent will wait for this submit to complete
      }
      else
         submitInfo.signalSemaphoreCount = 0;

      submitInfo.pWaitDstStageMask = &stageFlags;
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = commandBuffer->GetVkHandlePtr();

      if (renderFence == nullptr)
         Debug::ErrorCheck(vkQueueSubmit(GetVkHandle(), 1, &submitInfo, VK_NULL_HANDLE));
      else
         Debug::ErrorCheck(vkQueueSubmit(GetVkHandle(), 1, &submitInfo, renderFence->GetVkHandle()));
   }

   void Queue::WaitIdle()
   {
      Debug::ErrorCheck(vkQueueWaitIdle(GetVkHandle()));
   }
}