#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include "Handle.h"

namespace VulkanLib
{
	class Device;
	class DescriptorSet;

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
		PipelineLayout(Device* device, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, PushConstantRange* pushConstantRage);

		void Create(VkDescriptorSetLayout* setLayout, PushConstantRange* pushConstantRage);
	private:
	};
}
