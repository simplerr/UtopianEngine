#include "vulkan/Debug.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/CommandBuffer.h"
#include "QueryPoolStatistics.h"

namespace Utopian::Vk
{
	QueryPoolStatistics::QueryPoolStatistics(Device* device)
		: Handle(device, vkDestroyQueryPool)
	{
		VkQueryPoolCreateInfo queryPoolInfo = {};
		queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		queryPoolInfo.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
		queryPoolInfo.pipelineStatistics =
			VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
			VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
			VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
			VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT |
			VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT |
			VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
			VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT |
			VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
			VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT |
			VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT |
			VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;
		queryPoolInfo.queryCount = StatisticsIndex::NUM_STATISTICS;
		Debug::ErrorCheck(vkCreateQueryPool(device->GetVkDevice(), &queryPoolInfo, NULL, &mHandle));
	}

	void QueryPoolStatistics::RetreiveResults()
	{
		vkGetQueryPoolResults(
			GetDevice()->GetVkDevice(),
			mHandle,
			0,
			1,
			mStatistics.size() * sizeof(uint64_t),
			mStatistics.data(),
			sizeof(uint64_t),
			VK_QUERY_RESULT_64_BIT);
	}

	uint64_t QueryPoolStatistics::GetStatistics(StatisticsIndex index)
	{
		return mStatistics[index];
	}

	void QueryPoolStatistics::Begin(CommandBuffer* commandBuffer)
	{
		VkCommandBuffer cmdBuffer = commandBuffer->GetVkHandle();
		vkCmdBeginQuery(cmdBuffer, mHandle, 0, 0);
	}

	void QueryPoolStatistics::End(CommandBuffer* commandBuffer)
	{
		VkCommandBuffer cmdBuffer = commandBuffer->GetVkHandle();
		vkCmdEndQuery(cmdBuffer, mHandle, 0);
	}

	void QueryPoolStatistics::Reset(CommandBuffer* commandBuffer)
	{
		VkCommandBuffer cmdBuffer = commandBuffer->GetVkHandle();
		vkCmdResetQueryPool(cmdBuffer, mHandle, 0, mStatistics.size());
	}
}
