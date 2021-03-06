#pragma once

#include "Handle.h"
#include "vulkan/VulkanPrerequisites.h"

namespace Utopian::Vk
{
   /** Wrapper for VkSemaphore. */
   class Semaphore : public Handle<VkSemaphore>
   {
   public:
      Semaphore(Device* device);
   private:
   };
}
