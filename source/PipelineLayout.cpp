#include "PipelineLayout.h"
#include "VulkanDebug.h"

namespace VulkanLib
{
	PipelineLayout::PipelineLayout()
		: Handle(vkDestroyPipelineLayout)
	{

	}

	void PipelineLayout::Create(VkDevice device, VkDescriptorSetLayout* setLayout, PushConstantRange* pushConstantRage)
	{
		VkPipelineLayoutCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createInfo.pNext = NULL;
		createInfo.setLayoutCount = 1;
		createInfo.pSetLayouts = setLayout;
		createInfo.pushConstantRangeCount = 1;
		createInfo.pPushConstantRanges = &pushConstantRage->pushConstantRange;

		VulkanDebug::ErrorCheck(vkCreatePipelineLayout(device, &createInfo, nullptr, &mHandle));
	}
}
