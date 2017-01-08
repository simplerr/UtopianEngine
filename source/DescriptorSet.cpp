#include "DescriptorSet.h"

namespace VulkanLib
{
	void DescriptorSet::Cleanup(VkDevice device)
	{
		vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
	}

	void DescriptorSet::AddLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags)
	{
		VkDescriptorSetLayoutBinding layoutBinding = {
			binding,
			descriptorType,
			descriptorCount,
			stageFlags
		};

		mLayoutBindings.push_back(layoutBinding);
	}

	void DescriptorSet::CreateLayout(VkDevice device)
	{
		// One more binding would be used for a texture (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER and VK_SHADER_STAGE_FRAGMENT_BIT)
		VkDescriptorSetLayoutCreateInfo createInfo = CreateInfo::DescriptorSetLayout(mLayoutBindings.size(), mLayoutBindings.data());
		VulkanDebug::ErrorCheck(vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &setLayout));
	}

	void DescriptorSet::AllocateDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool)
	{
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &setLayout;	// allocInfo.pSetLayouts = &mDescriptorSetLayout;

		VulkanDebug::ErrorCheck(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));
	}

	void DescriptorSet::BindUniformBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
	{
		VkWriteDescriptorSet writeDescriptorSet = {};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = descriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.pBufferInfo = bufferInfo;
		writeDescriptorSet.dstBinding = binding;				// Binds this uniform buffer to binding point 0

		mWriteDescriptorSets.push_back(writeDescriptorSet);
	}

	void DescriptorSet::BindCombinedImage(uint32_t binding, VkDescriptorImageInfo* imageInfo)
	{
		VkWriteDescriptorSet writeDescriptorSet = {};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = descriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet.pImageInfo = imageInfo;
		writeDescriptorSet.dstBinding = binding;				

		mWriteDescriptorSets.push_back(writeDescriptorSet);
	}

	void DescriptorSet::UpdateUniformBuffer(uint32_t binding, VkDescriptorBufferInfo * bufferInfo)
	{
		// [TODO]
	}

	void DescriptorSet::UpdateCombinedImage(uint32_t binding, VkDescriptorImageInfo * imageInfo)
	{
		// [TODO]
	}

	std::vector<VkDescriptorSetLayoutBinding> DescriptorSet::GetLayoutBindings()
	{
		return mLayoutBindings;
	}

	void DescriptorSet::UpdateDescriptorSets(VkDevice device)
	{
		vkUpdateDescriptorSets(device, mWriteDescriptorSets.size(), mWriteDescriptorSets.data(), 0, NULL);
	}
}