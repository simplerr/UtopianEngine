#include "vulkan/Debug.h"
#include "vulkan/handles/Device.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/Effect.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Sampler.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"

namespace Utopian::Vk
{
	DescriptorSet::DescriptorSet(Device* device, DescriptorSetLayout* setLayout, DescriptorPool* descriptorPool)
	{
		Create(device, setLayout, descriptorPool);
	}

	DescriptorSet::DescriptorSet(Device* device, Effect* effect, uint32_t set, DescriptorPool* descriptorPool)
	{
		mShader = effect->GetShader();

		Create(device, effect->GetPipelineInterface()->GetDescriptorSetLayout(set), descriptorPool);
	}

	DescriptorSet::~DescriptorSet()
	{
		// The descriptor set should be freed once the descriptor pool is released
	}

	void DescriptorSet::Create(Device* device, DescriptorSetLayout* setLayout, DescriptorPool* descriptorPool)
	{
		mDevice = device;
		mSetLayout = (DescriptorSetLayout*)setLayout;

		VkDescriptorSetLayout setLayoutVk = setLayout->GetVkHandle();

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool->GetVkHandle();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &setLayoutVk;

		Debug::ErrorCheck(vkAllocateDescriptorSets(mDevice->GetVkDevice(), &allocInfo, &mDescriptorSet));
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
			writeDescriptorSet.dstSet = mDescriptorSet;
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
			writeDescriptorSet.dstSet = mDescriptorSet;
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
		writeDescriptorSet.dstSet = mDescriptorSet;
		writeDescriptorSet.descriptorCount = descriptorCount;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet.pImageInfo = imageInfo;
		writeDescriptorSet.dstBinding = binding;				

		mWriteDescriptorSets.push_back(writeDescriptorSet);
	}

	void DescriptorSet::BindCombinedImage(uint32_t binding, VkImageView imageView, Sampler* sampler, VkImageLayout imageLayout)
	{
		/* Check if the VkDescriptorImageInfo already is added to the map.
		Letting DescriptorSet handle the VkDescriptorImageInfo makes decouples
		Image and Sampler from each other. The same Image should be able to use with
		different samples and vice versa.
		*/
		if (mImageInfoMap.find(binding) != mImageInfoMap.end())
		{
			mImageInfoMap[binding].sampler = sampler->GetVkHandle();
			mImageInfoMap[binding].imageView = imageView;
		}
		else
		{
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.sampler = sampler->GetVkHandle();
			imageInfo.imageView = imageView;
			imageInfo.imageLayout = imageLayout; // Default is VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL

			mImageInfoMap[binding] = imageInfo;
		}

		VkWriteDescriptorSet writeDescriptorSet = {};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = mDescriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet.pImageInfo = &mImageInfoMap[binding];
		writeDescriptorSet.dstBinding = binding;

		mWriteDescriptorSets.push_back(writeDescriptorSet);
	}

	void DescriptorSet::BindCombinedImage(uint32_t binding, Image* image, Sampler* sampler)
	{
		BindCombinedImage(binding, image->GetView(), sampler, image->GetFinalLayout());
	}

	void DescriptorSet::BindUniformBuffer(std::string name, VkDescriptorBufferInfo* bufferInfo)
	{
		assert(mShader != nullptr);
		BindUniformBuffer(mShader->NameToBinding(name), bufferInfo);
	}

	void DescriptorSet::BindStorageBuffer(std::string name, VkDescriptorBufferInfo* bufferInfo)
	{
		assert(mShader != nullptr);
		BindStorageBuffer(mShader->NameToBinding(name), bufferInfo);
	}

	void DescriptorSet::BindCombinedImage(std::string name, VkDescriptorImageInfo* imageInfo, uint32_t descriptorCount)
	{
		assert(mShader != nullptr);
		BindCombinedImage(mShader->NameToBinding(name), imageInfo, descriptorCount);
	}

	void DescriptorSet::BindCombinedImage(std::string name, Image* image, Sampler* sampler)
	{
		assert(mShader != nullptr);
		BindCombinedImage(mShader->NameToBinding(name), image, sampler);
	}

	void DescriptorSet::BindCombinedImage(std::string name, VkImageView imageView, Sampler* sampler)
	{
		assert(mShader != nullptr);
		BindCombinedImage(mShader->NameToBinding(name), imageView, sampler);
	}

	void DescriptorSet::UpdateDescriptorSets()
	{
		vkUpdateDescriptorSets(mDevice->GetVkDevice(), mWriteDescriptorSets.size(), mWriteDescriptorSets.data(), 0, NULL);
	}

	VkDescriptorSet DescriptorSet::GetVkHandle() const
	{
		return mDescriptorSet;
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

		if (maxSets > 0)
		{
			VkDescriptorPoolCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			createInfo.maxSets = maxSets;
			createInfo.poolSizeCount = mDescriptorSizes.size();
			createInfo.pPoolSizes = mDescriptorSizes.data();
			createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

			Debug::ErrorCheck(vkCreateDescriptorPool(GetDevice()->GetVkDevice(), &createInfo, nullptr, &mHandle));
		}
	}
}