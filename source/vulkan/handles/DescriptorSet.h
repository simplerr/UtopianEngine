#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "vulkan/VulkanHelpers.h"

namespace VulkanLib
{
	class DescriptorSetLayout;

	/*
	Wraps VkDescriptorSetLayout and VkDescriptorSet to make them easier to work with
	*/
	class DescriptorSet
	{
	public:
		DescriptorSet(DescriptorSetLayout* setLayout);
		void Cleanup(VkDevice device);

		void AllocateDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool);
		void UpdateDescriptorSets(VkDevice device);

		void BindUniformBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		void BindCombinedImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

		void UpdateUniformBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		void UpdateCombinedImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);	// Will be used for changing the texture

		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	private:
		DescriptorSetLayout* mSetLayout;
		std::vector<VkWriteDescriptorSet> mWriteDescriptorSets;
	};

	/*
	Wraps VkDescriptorPool to make it easier to work with
	Can be created directly from a descriptor layout vector
	*/
	class DescriptorPool
	{
	public:
		void Cleanup(VkDevice device);
		void AddDescriptor(VkDescriptorType type, uint32_t count);
		void CreatePool(VkDevice device);
		VkDescriptorPool GetVkDescriptorPool();
	private:
		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorPoolSize> mDescriptorSizes;
	};
}
