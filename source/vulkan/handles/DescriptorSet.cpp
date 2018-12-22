#include "vulkan/VulkanDebug.h"
#include "vulkan/Device.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/handles/Effect.h"
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

		VkDescriptorSetLayout setLayoutVk = mSetLayout->GetVkHandle();

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool->GetVkHandle();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &setLayoutVk;

		VulkanDebug::ErrorCheck(vkAllocateDescriptorSets(mDevice->GetVkDevice(), &allocInfo, &descriptorSet));
	}

	DescriptorSet::DescriptorSet(Device* device, Effect* pipeline, uint32_t set, DescriptorPool* descriptorPool)
	{
		Create(device, pipeline, set, descriptorPool);
	}

	DescriptorSet::DescriptorSet()
	{
		mDevice = nullptr;
		mSetLayout = nullptr;
	}

	DescriptorSet::~DescriptorSet()
	{
		//vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
	}

	void DescriptorSet::Create(Device* device, Effect* pipeline, uint32_t set, DescriptorPool* descriptorPool)
	{
		mDevice = device;
		mSetLayout = pipeline->GetPipelineInterface()->GetDescriptorSetLayout(set);
		mShader = pipeline->GetShader();

		VkDescriptorSetLayout setLayoutVk = mSetLayout->GetVkHandle();

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool->GetVkHandle();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &setLayoutVk;

		VulkanDebug::ErrorCheck(vkAllocateDescriptorSets(mDevice->GetVkDevice(), &allocInfo, &descriptorSet));
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

	void DescriptorSet::BindCombinedImage(uint32_t binding, VkDescriptorImageInfo* imageInfo, uint32_t descriptorCount)
	{
		VkWriteDescriptorSet writeDescriptorSet = {};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = descriptorSet;
		writeDescriptorSet.descriptorCount = descriptorCount;
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

	void DescriptorSet::BindUniformBuffer(std::string name, VkDescriptorBufferInfo* bufferInfo)
	{
		BindUniformBuffer(mShader->NameToBinding(name), bufferInfo);
	}

	void DescriptorSet::BindStorageBuffer(std::string name, VkDescriptorBufferInfo* bufferInfo)
	{
		BindStorageBuffer(mShader->NameToBinding(name), bufferInfo);
	}

	void DescriptorSet::BindCombinedImage(std::string name, VkDescriptorImageInfo* imageInfo, uint32_t descriptorCount)
	{
		BindCombinedImage(mShader->NameToBinding(name), imageInfo, descriptorCount);
	}

	void DescriptorSet::BindCombinedImage(std::string name, Image* image, Sampler* sampler)
	{
		BindCombinedImage(mShader->NameToBinding(name), image, sampler);
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

	DescriptorPool::DescriptorPool()
		: Handle(nullptr, vkDestroyDescriptorPool)
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
		Create(GetDevice());
	}

	void DescriptorPool::Create(Device* device)
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

		VulkanDebug::ErrorCheck(vkCreateDescriptorPool(device->GetVkDevice(), &createInfo, nullptr, &mHandle));
	}
}