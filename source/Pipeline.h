#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include "Handle.h"

namespace VulkanLib
{
	class VertexDescription;

	class Pipeline : public Handle<VkPipeline>
	{
	public:
		Pipeline();

		void Create(VkDevice device, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, VertexDescription* vertexDescription, const std::array<VkPipelineShaderStageCreateInfo, 2>& shaderStages);
	private:
	};
}
