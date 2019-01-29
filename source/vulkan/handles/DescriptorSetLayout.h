#pragma once

#include <vector>
#include "Handle.h" 
#include "vulkan/VulkanInclude.h"

namespace Utopian::Vk
{
	/** Wrapper for VkDescriptorSetLayout. */
	class DescriptorSetLayout : public Handle<VkDescriptorSetLayout>
	{
	public:
		DescriptorSetLayout(Device* device);

		/** Adds a descriptor type to the layout. */
		void AddBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags);

		/** Helper functions to make adding different descriptor types easiser. */
		void AddUniformBuffer(uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);
		void AddStorageBuffer(uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);
		void AddCombinedImageSampler(uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);

		/** 
		 * Creates the descriptor set layout. 
		 * @note This must be called after all needed descriptor
		 * types have been added to the layout.
		 */
		void Create();
	private:
		std::vector<VkDescriptorSetLayoutBinding> mLayoutBindings;
	};
}

