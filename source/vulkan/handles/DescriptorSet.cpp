#include "vulkan/VulkanDebug.h"
#include "vulkan/Device.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"

namespace Vulkan
{
	DescriptorSet::DescriptorSet(Device* device, DescriptorSetLayout* setLayout, DescriptorPool* descriptorPool)
	{
		mDevice = device;
		mSetLayout = setLayout;
		mDescriptorPool = descriptorPool;
	}

	void DescriptorSet::Cleanup(VkDevice device)
	{
		//vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
	}

	void DescriptorSet::AllocateDescriptorSets()
	{
		VkDescriptorSetLayout setLayout = mSetLayout->GetVkHandle();
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptorPool->GetVkHandle();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &setLayout;	

		VulkanDebug::ErrorCheck(vkAllocateDescriptorSets(mDevice->GetVkDevice(), &allocInfo, &descriptorSet));
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

	void DescriptorSet::UpdateUniformBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
	{
		// [TODO]
	}

	void DescriptorSet::UpdateCombinedImage(uint32_t binding, VkDescriptorImageInfo* imageInfo)
	{
		// [TODO]
	}

	void DescriptorSet::UpdateDescriptorSets()
	{
		vkUpdateDescriptorSets(mDevice->GetVkDevice(), mWriteDescriptorSets.size(), mWriteDescriptorSets.data(), 0, NULL);
	}

	DescriptorPool::DescriptorPool(Device* device)
		: Handle(device, vkDestroyDescriptorPool)
	{

	}

	void DescriptorPool::AddDescriptor(VkDescriptorType type, uint32_t count)
	{
		VkDescriptorPoolSize descriptorSize = {};
		descriptorSize.type = type;
		descriptorSize.descriptorCount = count;
		mDescriptorSizes.push_back(descriptorSize);
	}

	void DescriptorPool::Create()
	{
		uint32_t maxSets = 0;
		for (auto descriptorSize : mDescriptorSizes) 
		{
			maxSets += descriptorSize.descriptorCount;
		}

		VkDescriptorPoolCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createInfo.maxSets = maxSets;
		createInfo.poolSizeCount = mDescriptorSizes.size();
		createInfo.pPoolSizes = mDescriptorSizes.data();

		VulkanDebug::ErrorCheck(vkCreateDescriptorPool(GetDevice(), &createInfo, nullptr, &mHandle));
	}
}