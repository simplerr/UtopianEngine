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

   float QueryPoolTimestamp::GetStartTimestamp()
   {
      uint64_t startTimestamp;
      Debug::ErrorCheck(vkGetQueryPoolResults(GetVkDevice(), mHandle, 0, 1,
                                              sizeof(uint64_t), &startTimestamp, sizeof(uint64_t),
                                              VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));
   
      return startTimestamp / 1e6;
   }

   float QueryPoolTimestamp::GetEndTimestamp()
   {
      uint64_t endTimestamp;
      Debug::ErrorCheck(vkGetQueryPoolResults(GetVkDevice(), mHandle, 1, 1,
                                              sizeof(uint64_t), &endTimestamp, sizeof(uint64_t),
                                              VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));
      return endTimestamp / 1e6;
   }

   float QueryPoolTimestamp::GetElapsedTime()
   {
      float delta = GetEndTimestamp() - GetStartTimestamp();
      return delta;
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
