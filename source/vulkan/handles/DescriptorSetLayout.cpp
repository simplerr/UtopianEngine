#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/VulkanDebug.h"

namespace Vulkan
{
	DescriptorSetLayout::DescriptorSetLayout(Device* device)
		: Handle(device, vkDestroyDescriptorSetLayout)
	{

	}

	DescriptorSetLayout::DescriptorSetLayout()
	{

	}

	void DescriptorSetLayout::AddBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags)
	{
		VkDescriptorSetLayoutBinding layoutBinding = {};
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.descriptorCount = descriptorCount;
		layoutBinding.stageFlags = stageFlags;

		mLayoutBindings.push_back(layoutBinding);
	}

	void DescriptorSetLayout::AddUniformBuffer(uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
	{
		AddBinding(binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorCount, stageFlags);
	}

	void DescriptorSetLayout::AddStorageBuffer(uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
	{
		AddBinding(binding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptorCount, stageFlags);
	}

	void DescriptorSetLayout::AddCombinedImageSampler(uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
	{
		AddBinding(binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorCount, stageFlags);
	}

	void DescriptorSetLayout::Create()
	{
		VkDescriptorSetLayoutCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = mLayoutBindings.size();
		createInfo.pBindings = mLayoutBindings.data();

		VulkanDebug::ErrorCheck(vkCreateDescriptorSetLayout(GetDevice(), &createInfo, nullptr, &mHandle));
	}

	void DescriptorSetLayout::Create(Device* device)
	{
		VkDescriptorSetLayoutCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = mLayoutBindings.size();
		createInfo.pBindings = mLayoutBindings.data();

		VulkanDebug::ErrorCheck(vkCreateDescriptorSetLayout(device->GetVkDevice(), &createInfo, nullptr, &mHandle));
	}
}