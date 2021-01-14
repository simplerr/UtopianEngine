#pragma once

#include "Handle.h" 
#include "vulkan/VulkanPrerequisites.h"
#include <array>

namespace Utopian::Vk
{
   /** Wrapper for VkQueryPool. */
   class QueryPoolTimestamp : public Handle<VkQueryPool>
   {
   public:
      QueryPoolTimestamp(Device* device);

      void Begin(CommandBuffer* commandBuffer);
      void End(CommandBuffer* commandBuffer);
      void Reset(CommandBuffer* commandBuffer);

      /** Returns the elapsed time in milliseconds. */
      float GetElapsedTime();

   private:
      const uint32_t NUM_QUERIES_PER_POOL = 2u;
   };
}
