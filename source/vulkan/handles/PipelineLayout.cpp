#include "vulkan/VulkanDebug.h"
#include "vulkan/handles/DescriptorSet.h"
#include "PipelineLayout.h"

namespace VulkanLib
{
	PipelineLayout::PipelineLayout(Device* device, VkDescriptorSetLayout* setLayout, PushConstantRange* pushConstantRage)
		: Handle(device, vkDestroyPipelineLayout)
	{
		Create(setLayout, pushConstantRage);
	}

	void PipelineLayout::Create(VkDescriptorSetLayout* setLayout, PushConstantRange* pushConstantRage)
	{
		VkPipelineLayoutCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createInfo.pNext = NULL;
		createInfo.setLayoutCount = 1;
		createInfo.pSetLayouts = setLayout;
		createInfo.pushConstantRangeCount = 1;
		createInfo.pPushConstantRanges = &pushConstantRage->pushConstantRange;

		VulkanDebug::ErrorCheck(vkCreatePipelineLayout(GetDevice(), &createInfo, nullptr, &mHandle));
	}

	PipelineLayout::PipelineLayout(Device* device, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, PushConstantRange* pushConstantRage)
		: Handle(device, vkDestroyPipelineLayout)
	{
		const VkDescriptorSetLayout layouts[] = { descriptorSetLayouts[0], descriptorSetLayouts[1], descriptorSetLayouts[2] };
		VkPipelineLayoutCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createInfo.pNext = NULL;
		createInfo.setLayoutCount = 3;// descriptorSetLayouts.size();
		createInfo.pSetLayouts = layouts;// descriptorSetLayouts.data();
		createInfo.pushConstantRangeCount = 1;
		createInfo.pPushConstantRanges = &pushConstantRage->pushConstantRange;

		VulkanDebug::ErrorCheck(vkCreatePipelineLayout(GetDevice(), &createInfo, nullptr, &mHandle));
	}
}
