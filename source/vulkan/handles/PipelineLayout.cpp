#include "vulkan/VulkanDebug.h"
#include "PipelineLayout.h"

namespace VulkanLib
{
	PipelineLayout::PipelineLayout(VkDevice device, VkDescriptorSetLayout* setLayout, PushConstantRange* pushConstantRage)
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
}
