#pragma once

#include <vulkan/vulkan.h>
#include "Handle.h"

namespace VulkanLib
{
	class Device;

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
		PipelineLayout(Device* device, VkDescriptorSetLayout* setLayout, PushConstantRange* pushConstantRage);

		void Create(VkDescriptorSetLayout* setLayout, PushConstantRange* pushConstantRage);
	private:
	};
}
