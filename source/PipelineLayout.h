#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include "Handle.h"

namespace VulkanLib
{
	struct PushConstantRange
	{
		PushConstantRange(VkShaderStageFlagBits shaderStage, uint32_t size, uint32_t offset = 0)
		{
			pushConstantRange.stageFlags = shaderStage;
			pushConstantRange.size = size;
			pushConstantRange.offset = offset;
		}

		VkPushConstantRange pushConstantRange = {};
	};
	class PipelineLayout : public Handle<VkPipelineLayout>
	{
	public:
		PipelineLayout();

		void Create(VkDevice device, VkDescriptorSetLayout* setLayout, PushConstantRange* pushConstantRage);
	private:
	};
}
