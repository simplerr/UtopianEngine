#include "vulkan/Debug.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/CommandBuffer.h"
#include "QueryPoolTimestamp.h"

namespace Utopian::Vk
{
   QueryPoolTimestamp::QueryPoolTimestamp(Device* device)
      : Handle(device, vkDestroyQueryPool)
   {
      VkQueryPoolCreateInfo queryPoolInfo = {};
      queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
      queryPoolInfo.pNext = nullptr;
      queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
      queryPoolInfo.pipelineStatistics = 0;
      queryPoolInfo.queryCount = NUM_QUERIES_PER_POOL;
      Debug::ErrorCheck(vkCreateQueryPool(device->GetVkDevice(), &queryPoolInfo, NULL, &mHandle));
   }

   float QueryPoolTimestamp::GetElapsedTime()
   {
      std::array<uint64_t, 2> timestamps;
      Debug::ErrorCheck(vkGetQueryPoolResults(GetVkDevice(), mHandle, 0, 2,
                                              2 * sizeof(uint64_t), &timestamps, sizeof(uint64_t),
                                              VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

      float timestampPeriod = GetDevice()->GetProperties().limits.timestampPeriod;
      float duration = ((timestamps[1] - timestamps[0]) * timestampPeriod) / NS_PER_MS;

      return duration;
   }

   void QueryPoolTimestamp::Begin(CommandBuffer* commandBuffer)
   {
      vkCmdWriteTimestamp(commandBuffer->GetVkHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, mHandle, 0);
   }

   void QueryPoolTimestamp::End(CommandBuffer* commandBuffer)
   {
      vkCmdWriteTimestamp(commandBuffer->GetVkHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, mHandle, 1);
   }

   void QueryPoolTimestamp::Reset(CommandBuffer* commandBuffer)
   {
      vkCmdResetQueryPool(commandBuffer->GetVkHandle(), mHandle, 0, NUM_QUERIES_PER_POOL);
   }
}
