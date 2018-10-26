#include "vulkan/PipelineInterface.h"
#include "vulkan/Device.h"
#include "vulkan/VulkanDebug.h"

namespace Utopian::Vk
{
	void PipelineInterface::CreateLayouts(Device* device)
	{
		// Create all VkDescriptorSetLayouts
		for (auto& setLayout : mDescriptorSetLayouts)
		{
			setLayout.second.Create(device);
		}

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		for (auto& setLayout : mDescriptorSetLayouts)
		{
			descriptorSetLayouts.push_back(setLayout.second.GetVkHandle());
		}

		// Create a VkPipelineLayout from all the VkDescriptorSetLayouts and Push Constant ranges
		VkPipelineLayoutCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createInfo.pNext = NULL;
		createInfo.setLayoutCount = descriptorSetLayouts.size();
		createInfo.pSetLayouts = descriptorSetLayouts.data();

		if (mPushConstantRanges.size() > 0)
		{
			createInfo.pushConstantRangeCount = mPushConstantRanges.size();
			createInfo.pPushConstantRanges = mPushConstantRanges.data();
		}

		VulkanDebug::ErrorCheck(vkCreatePipelineLayout(device->GetVkDevice(), &createInfo, nullptr, &mPipelineLayout));
	}

	void PipelineInterface::AddUniformBuffer(uint32_t descriptorSet, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
	{
		AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorSet, binding, descriptorCount, stageFlags);
	}

	void PipelineInterface::AddStorageBuffer(uint32_t descriptorSet, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
	{
		AddDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptorSet, binding, descriptorCount, stageFlags);
	}

	void PipelineInterface::AddCombinedImageSampler(uint32_t descriptorSet, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
	{
		AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorSet, binding, descriptorCount, stageFlags);
	}

	void PipelineInterface::AddPushConstantRange(uint32_t size, VkShaderStageFlags shaderStage, uint32_t offset)
	{
		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.stageFlags = shaderStage;
		pushConstantRange.size = size;
		pushConstantRange.offset = offset;

		mPushConstantRanges.push_back(pushConstantRange);
	}

	DescriptorSetLayout* PipelineInterface::GetDescriptorSetLayout(uint32_t descriptorSet)
	{
		return &mDescriptorSetLayouts[descriptorSet];
	}

	VkDescriptorSetLayout PipelineInterface::GetVkDescriptorSetLayout(uint32_t descriptorSet)
	{
		return mDescriptorSetLayouts[descriptorSet].GetVkHandle();
	}

	VkPipelineLayout PipelineInterface::GetPipelineLayout()
	{
		return mPipelineLayout;
	}

	void PipelineInterface::AddDescriptor(VkDescriptorType descriptorType, uint32_t descriptorSet, uint32_t binding, uint32_t descriptorCount, VkShaderStageFlags stageFlags)
	{
		if (mDescriptorSetLayouts.find(descriptorSet) == mDescriptorSetLayouts.end())
		{
			mDescriptorSetLayouts[descriptorSet] = DescriptorSetLayout();
		}

		mDescriptorSetLayouts[descriptorSet].AddBinding(binding, descriptorType, descriptorCount, stageFlags);
	}

	uint32_t PipelineInterface::GetNumDescriptorSets() const
	{
		return mDescriptorSetLayouts.size();
	}
}