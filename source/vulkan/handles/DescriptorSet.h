#pragma once

#include <vector>
#include <map>
#include "vulkan/handles/Handle.h"
#include "vulkan/VulkanInclude.h"

namespace Vulkan
{
	/*
	Wraps VkDescriptorSetLayout and VkDescriptorSet to make them easier to work with
	*/
	class DescriptorSet
	{
	public:
		DescriptorSet(Device* device, DescriptorSetLayout* setLayout, DescriptorPool* descriptorPool);
		~DescriptorSet();

		void UpdateDescriptorSets();

		void BindUniformBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		void BindStorageBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		void BindCombinedImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);
		void BindCombinedImage(uint32_t binding, Image* image, Sampler* sampler);

		// NOTE: TODO: Legacy
		void UpdateCombinedImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);	// Will be used for changing the texture

		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

	private:
		Device* mDevice;
		DescriptorSetLayout* mSetLayout;
		DescriptorPool* mDescriptorPool;
		std::vector<VkWriteDescriptorSet> mWriteDescriptorSets;
		std::map<int, VkDescriptorImageInfo> mImageInfoMap;
	};

	/*
	Wraps VkDescriptorPool to make it easier to work with
	Can be created directly from a descriptor layout vector
	*/
	class DescriptorPool : public Handle<VkDescriptorPool>
	{
	public:
		DescriptorPool(Device* device);
		void AddDescriptor(VkDescriptorType type, uint32_t count);
		void Create();
	private:
		std::vector<VkDescriptorPoolSize> mDescriptorSizes;
	};
}
