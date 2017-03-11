#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "vulkan/VulkanHelpers.h"
#include "vulkan/handles/Handle.h"

namespace Vulkan
{
	class DescriptorSetLayout;
	class DescriptorPool;
	class Device;

	/*
	Wraps VkDescriptorSetLayout and VkDescriptorSet to make them easier to work with
	*/
	class DescriptorSet
	{
	public:
		DescriptorSet(Device* device, DescriptorSetLayout* setLayout, DescriptorPool* descriptorPool);
		void Cleanup(VkDevice device);

		void AllocateDescriptorSets();
		void UpdateDescriptorSets();

		void BindUniformBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		void BindStorageBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		void BindCombinedImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

		void UpdateUniformBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		void UpdateCombinedImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);	// Will be used for changing the texture

		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	private:
		Device* mDevice;
		DescriptorSetLayout* mSetLayout;
		DescriptorPool* mDescriptorPool;
		std::vector<VkWriteDescriptorSet> mWriteDescriptorSets;
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
