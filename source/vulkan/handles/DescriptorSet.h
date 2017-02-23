#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "vulkan/VulkanHelpers.h"

namespace VulkanLib
{
	/*
	Wraps VkDescriptorSetLayout and VkDescriptorSet to make them easier to work with
	*/
	class DescriptorSet
	{
	public:
		void Cleanup(VkDevice device);

		void AddLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags);
		void CreateLayout(VkDevice device);

		void AllocateDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool);
		void UpdateDescriptorSets(VkDevice device);

		void BindUniformBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		void BindCombinedImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

		void UpdateUniformBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		void UpdateCombinedImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);	// Will be used for changing the texture

		std::vector<VkDescriptorSetLayoutBinding> GetLayoutBindings();
		// Add more binding function when needed...	

		// Public for ease of use
		VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

		std::vector<VkDescriptorSetLayoutBinding> mLayoutBindings;
	private:
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
		void CreatePoolFromLayout(VkDevice device, std::vector<VkDescriptorSetLayoutBinding>& descriptorLayouts);
		void AddDescriptor(VkDescriptorType type, uint32_t count);
		void CreatePool(VkDevice device);
		VkDescriptorPool GetVkDescriptorPool();
	private:
		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorPoolSize> mDescriptorSizes;
	};
}
