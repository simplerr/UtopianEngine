#pragma once

#include <vector>
#include "Handle.h"
#include "vulkan/VulkanInclude.h"

namespace Utopian::Vk
{
	class PipelineLayout : public Handle<VkPipelineLayout>
	{
	public:
		//PipelineLayout(Device* device, VkDescriptorSetLayout* setLayout, PushConstantRange* pushConstantRage);
		PipelineLayout(Device* device);

		void AddDescriptorSetLayout(DescriptorSetLayout* descriptorSetLayout);
		void AddPushConstantRange(VkShaderStageFlags shaderStage, uint32_t size, uint32_t offset = 0);
		void Create();

	private:
		std::vector<VkDescriptorSetLayout> mDescriptorSetLayouts;
		VkPushConstantRange mPushConstantRange = {};
		bool mPushConstantActive = false;
	};
}
