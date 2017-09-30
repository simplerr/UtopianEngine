#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <map>
#include "vulkan/handles/DescriptorSetLayout.h"

namespace Vulkan
{
	class PipelineLayout;
	class Device;
	class DescriptorSetLayout;

	class PipelineInterface
	{
	public:
		/*
			Creates the descriptor set layout and pipeline layout objects	
			\note Must be called before calling Create()
		*/
		void CreateLayouts(Device* device);

		void AddUniformBuffer(uint32_t descriptorSet, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);
		void AddStorageBuffer(uint32_t descriptorSet, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);
		void AddCombinedImageSampler(uint32_t descriptorSet, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);
		void AddPushConstantRange(uint32_t size, VkShaderStageFlags shaderStage, uint32_t offset = 0);

		DescriptorSetLayout GetDescriptorSetLayout(uint32_t descriptorSet);
		VkDescriptorSetLayout GetVkDescriptorSetLayout(uint32_t descriptorSet);
		VkPipelineLayout GetPipelineLayout();

	private:
		void AddDescriptor(VkDescriptorType descriptorType, uint32_t descriptorSet, uint32_t binding, uint32_t descriptorCount, VkShaderStageFlags stageFlags);

		VkPipelineLayout mPipelineLayout;
		std::map<uint32_t, DescriptorSetLayout> mDescriptorSetLayouts;
		std::vector<VkPushConstantRange> mPushConstantRanges;
	};
}
