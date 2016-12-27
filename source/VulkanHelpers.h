#pragma once

#include <vulkan/vulkan.h>
#include "../base/vulkanTextureLoader.hpp"

namespace VulkanLib
{
	namespace CreateInfo
	{
		VkCommandPoolCreateInfo CommandPool(uint32_t queueFamily, VkCommandPoolCreateFlags flags);
		VkCommandBufferAllocateInfo CommandBuffer(VkCommandPool commandPool, VkCommandBufferLevel level, uint32_t count = 1);
		VkDescriptorPoolCreateInfo DescriptorPool(uint32_t poolSizeCount, const VkDescriptorPoolSize* pPoolSizes, uint32_t maxSets);
		VkDescriptorSetAllocateInfo DescriptorSet(VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSetLayout* pSetLayouts);
		VkDescriptorSetLayoutCreateInfo DescriptorSetLayout(uint32_t bindingCount, const VkDescriptorSetLayoutBinding* pBindings);
		VkPipelineLayoutCreateInfo PipelineLayout(uint32_t setLayoutCount, const VkDescriptorSetLayout* pSetLayouts);

	};

	VkDescriptorImageInfo GetTextureDescriptorInfo(vkTools::VulkanTexture texture);
}
