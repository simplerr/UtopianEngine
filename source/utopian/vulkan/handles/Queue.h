#pragma once

#include "Handle.h"
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/handles/Semaphore.h"
#include "utility/Common.h"

namespace Utopian::Vk
{
   /** Wrapper for VkQueue. */
   class Queue : public Handle<VkQueue>
   {
   public:
      Queue(Device* device);
      ~Queue();

      /**
       * Submits a recorded command buffer to the graphics queue.
       */
      void Submit(CommandBuffer* commandBuffer, Fence* renderFence, const SharedPtr<Semaphore>& waitSemaphore, const SharedPtr<Semaphore>& signalSemaphore);
      void WaitIdle();

   protected:
   };
}
