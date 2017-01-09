#pragma once
#include <vulkan/vulkan.h>
#include <array>

namespace VulkanLib
{
	class VertexDescription;

	class Pipeline
	{
	public:
		Pipeline();

		void CreatePipeline(VkDevice device, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, VertexDescription* vertexDescription, const std::array<VkPipelineShaderStageCreateInfo, 2>& shaderStages);
		void Cleanup(VkDevice device);
		VkPipeline GetVkPipeline();
	private:
		VkPipeline mVkPipeline;
	};
}
