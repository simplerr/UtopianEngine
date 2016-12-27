#include "VulkanHelpers.h"

namespace VulkanLib
{
	namespace CreateInfo
	{
		VkCommandPoolCreateInfo CommandPool(uint32_t queueFamily, VkCommandPoolCreateFlags flags)
		{
			// Setup thread command buffer pool
			VkCommandPoolCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			createInfo.queueFamilyIndex = queueFamily;									// NOTE: TODO: Need to store this as a member (Use Swapchain)!!!!!
			createInfo.flags = flags;

			return createInfo;
		}

		VkCommandBufferAllocateInfo CommandBuffer(VkCommandPool commandPool, VkCommandBufferLevel level, uint32_t count)
		{
			VkCommandBufferAllocateInfo allocateInfo = {};
			allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocateInfo.commandPool = commandPool;
			allocateInfo.commandBufferCount = count;
			allocateInfo.level = level;

			return allocateInfo;
		}

		VkDescriptorPoolCreateInfo DescriptorPool(uint32_t poolSizeCount, const VkDescriptorPoolSize * pPoolSizes, uint32_t maxSets)
		{
			VkDescriptorPoolCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			createInfo.poolSizeCount = poolSizeCount;
			createInfo.pPoolSizes = pPoolSizes;
			createInfo.maxSets = maxSets;

			return createInfo;
		}

		VkDescriptorSetAllocateInfo DescriptorSet(VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSetLayout* pSetLayouts)
		{
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;			// Has to have a separate descriptor pool for each thread!
			allocInfo.descriptorSetCount = descriptorSetCount;
			allocInfo.pSetLayouts = pSetLayouts;

			return allocInfo;
		}

		VkDescriptorSetLayoutCreateInfo DescriptorSetLayout(uint32_t bindingCount, const VkDescriptorSetLayoutBinding * pBindings)
		{
			VkDescriptorSetLayoutCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			createInfo.bindingCount = bindingCount;
			createInfo.pBindings = pBindings;

			return createInfo;
		}

		VkPipelineLayoutCreateInfo PipelineLayout(uint32_t setLayoutCount, const VkDescriptorSetLayout * pSetLayouts)
		{
			VkPipelineLayoutCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			createInfo.pNext = NULL;
			createInfo.setLayoutCount = setLayoutCount;
			createInfo.pSetLayouts = pSetLayouts;

			return createInfo;
		}
	}

	// Used when adding descriptor set bindings
	VkDescriptorImageInfo GetTextureDescriptorInfo(vkTools::VulkanTexture texture)
	{
		VkDescriptorImageInfo texDescriptor = {};
		texDescriptor.sampler = texture.sampler;
		texDescriptor.imageView = texture.view;
		texDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		return texDescriptor;
	}
}