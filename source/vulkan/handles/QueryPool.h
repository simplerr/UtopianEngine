#pragma once

#include "Handle.h" 
#include "vulkan/VulkanInclude.h"
#include <array>

namespace Utopian::Vk
{
	/** Wrapper for VkQueryPool. */
	class QueryPool : public Handle<VkQueryPool>
	{
	public:
		/** All available statistics. */
		enum StatisticsIndex
		{
			INPUT_ASSEMBLY_VERTICES_INDEX,
			INPUT_ASSEMBLY_PRIMITIVES_INDEX,
			VERTEX_SHADER_INVOCATIONS_INDEX,
			GEOMETRY_SHADER_INVOCATIONS_INDEX,
			GEOMETRY_SHADER_PRIMITIVES_INDEX,
			CLIPPING_INVOCATIONS_INDEX,
			CLIPPING_PRIMITIVES_INDEX,
			FRAGMENT_SHADER_INVOCATIONS_INDEX,
			TESSELLATION_CONTROL_SHADER_PATCHES_INDEX,
			TESSELLATION_EVALUATION_SHADER_INVOCATIONS_INDEX,
			COMPUTE_SHADER_INVOCATIONS_INDEX,
			NUM_STATISTICS
		};

		QueryPool(Device* device);
	
		/** 
		 * Retrieves the result of the submitted statistics query. 
		 * @note Must be called before calling GetStatistics().
		 * The results are not available immedietly. 
		 */
		void RetreiveResults();

		uint64_t GetStatistics(StatisticsIndex index);

		void Begin(CommandBuffer* commandBuffer);
		void End(CommandBuffer* commandBuffer);
		void Reset(CommandBuffer* commandBuffer);

	private:
		std::array <uint64_t, StatisticsIndex::NUM_STATISTICS> mStatistics;
	};
}
