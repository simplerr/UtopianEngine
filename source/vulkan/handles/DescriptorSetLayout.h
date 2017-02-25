#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "Handle.h" 

namespace VulkanLib
{
	class Device;

	class DescriptorSetLayout : public Handle<VkDescriptorSetLayout>
	{
	public:
		DescriptorSetLayout(Device* device);

		void AddBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags);
		void Create();
		VkDescriptorSetLayout GetLayout();
	private:
		std::vector<VkDescriptorSetLayoutBinding> mLayoutBindings;
		VkDescriptorSetLayout mSetLayout = VK_NULL_HANDLE;
	};
}

