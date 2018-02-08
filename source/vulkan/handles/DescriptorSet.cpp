#include "vulkan/VulkanDebug.h"
#include "vulkan/Device.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Sampler.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"

namespace Utopian::Vk
{
	DescriptorSet::DescriptorSet(Device* device, DescriptorSetLayout* setLayout, DescriptorPool* descriptorPool)
	{
		mDevice = device;
		mSetLayout = setLayout;
		mDescriptorPool = descriptorPool;

		VkDescriptorSetLayout setLayoutVk = mSetLayout->GetVkHandle();

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptorPool->GetVkHandle();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &setLayoutVk;

		VulkanDebug::ErrorCheck(vkAllocateDescriptorSets(mDevice->GetVkDevice(), &allocInfo, &descriptorSet));
	}

	DescriptorSet::~DescriptorSet()
	{
		//vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
	}

	void DescriptorSet::BindUniformBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
	{
		bool existing = false;
		for (int i = 0; i < mWriteDescriptorSets.size(); i++)
		{
			if (mWriteDescriptorSets[i].dstBinding == binding)
			{
				mWriteDescriptorSets[i].pBufferInfo = bufferInfo;
				existing = true;
			}
		}

		if (!existing)
		{
			VkWriteDescriptorSet writeDescriptorSet = {};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.dstSet = descriptorSet;
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSet.pBufferInfo = bufferInfo;
			writeDescriptorSet.dstBinding = binding;

			mWriteDescriptorSets.push_back(writeDescriptorSet);
		}
	}

	void DescriptorSet::BindStorageBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
	{
		bool existing = false;
		for (int i = 0; i < mWriteDescriptorSets.size(); i++)
		{
			if (mWriteDescriptorSets[i].dstBinding == binding)
			{
				mWriteDescriptorSets[i].pBufferInfo = bufferInfo;
				existing = true;
			}
		}

		if (!existing)
		{
			VkWriteDescriptorSet writeDescriptorSet = {};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.dstSet = descriptorSet;
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			writeDescriptorSet.pBufferInfo = bufferInfo;
			writeDescriptorSet.dstBinding = binding;

			mWriteDescriptorSets.push_back(writeDescriptorSet);
		}
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

	void DescriptorSet::BindCombinedImage(uint32_t binding, Image* image, Sampler* sampler)
	{
		/* Check if the VkDescriptorImageInfo already is added to the map.
		   Letting DescriptorSet handle the VkDescriptorImageInfo makes decouples
		   Image and Sampler from each other. The same Image should be able to use with
		   different samples and vice versa.
		*/
		if (mImageInfoMap.find(binding) != mImageInfoMap.end())
		{
			mImageInfoMap[binding].sampler = sampler->GetVkHandle();
			mImageInfoMap[binding].imageView = image->GetView();
		}
		else
		{
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.sampler = sampler->GetVkHandle();
			imageInfo.imageView = image->GetView();
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			mImageInfoMap[binding] = imageInfo;
		}

		VkWriteDescriptorSet writeDescriptorSet = {};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = descriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet.pImageInfo = &mImageInfoMap[binding];
		writeDescriptorSet.dstBinding = binding;				

		mWriteDescriptorSets.push_back(writeDescriptorSet);
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