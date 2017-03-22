#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "Handle.h" 

namespace Vulkan
{
	class Device;

	class DescriptorSetLayout : public Handle<VkDescriptorSetLayout>
	{
	public:
		DescriptorSetLayout(Device* device);
		DescriptorSetLayout();

		void AddBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags);
		void Create();
		void Create(Device* device);
	private:
		std::vector<VkDescriptorSetLayoutBinding> mLayoutBindings;
	};
}

