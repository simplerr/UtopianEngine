#include "vulkan/VulkanDebug.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "PipelineLayout.h"

namespace Utopian::Vk
{
	PipelineLayout::PipelineLayout(Device* device)
		: Handle(device, vkDestroyPipelineLayout)
	{

	}

	void PipelineLayout::AddDescriptorSetLayout(DescriptorSetLayout* descriptorSetLayout)
	{
		mDescriptorSetLayouts.push_back(descriptorSetLayout->GetVkHandle());
	}

	void PipelineLayout::AddPushConstantRange(VkShaderStageFlags shaderStage, uint32_t size, uint32_t offset)
	{
		mPushConstantRange.stageFlags = shaderStage;
		mPushConstantRange.size = size;
		mPushConstantRange.offset = offset;

		mPushConstantActive = true;
	}

	// TODO: Redundant and confusing, remove?
	void PipelineLayout::Create()
	{
		VkPipelineLayoutCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createInfo.pNext = NULL;
		createInfo.setLayoutCount = mDescriptorSetLayouts.size();
		createInfo.pSetLayouts = mDescriptorSetLayouts.data();

		if (mPushConstantActive)
		{
			createInfo.pushConstantRangeCount = 1;
			createInfo.pPushConstantRanges = &mPushConstantRange;
		}

		VulkanDebug::ErrorCheck(vkCreatePipelineLayout(GetVkDevice(), &createInfo, nullptr, &mHandle));
	}
}
