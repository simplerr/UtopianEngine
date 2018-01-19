#pragma once

#include <vector>
#include "Handle.h" 
#include "vulkan/VulkanInclude.h"

namespace Vulkan
{
	class DescriptorSetLayout : public Handle<VkDescriptorSetLayout>
	{
	public:
		DescriptorSetLayout(Device* device);
		DescriptorSetLayout();

		void AddBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags);

		void AddUniformBuffer(uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);
		void AddStorageBuffer(uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);
		void AddCombinedImageSampler(uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);

		void Create();
		void Create(Device* device);
	private:
		std::vector<VkDescriptorSetLayoutBinding> mLayoutBindings;
	};
}

