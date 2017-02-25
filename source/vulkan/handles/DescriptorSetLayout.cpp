#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/VulkanDebug.h"

namespace VulkanLib
{
	DescriptorSetLayout::DescriptorSetLayout(Device* device)
		: Handle(device, vkDestroyDescriptorSetLayout)
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

	void DescriptorSetLayout::Create()
	{
		VkDescriptorSetLayoutCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = mLayoutBindings.size();
		createInfo.pBindings = mLayoutBindings.data();

		VulkanDebug::ErrorCheck(vkCreateDescriptorSetLayout(GetDevice(), &createInfo, nullptr, &mSetLayout));
	}

	VkDescriptorSetLayout DescriptorSetLayout::GetLayout()
	{
		return mSetLayout;
	}
}