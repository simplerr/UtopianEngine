#pragma once

#include <vulkan/vulkan.h>
#include <array>
#include <map>
#include "Handle.h"
#include "vulkan/handles/DescriptorSetLayout.h"


namespace Vulkan
{
	class VertexDescription;
	class RenderPass;
	class PipelineLayout;
	class Device;
	class Shader;
	class DescriptorSetLayout;

	class Pipeline2 : public Handle<VkPipeline>
	{
	public:
		Pipeline2(Device* device, RenderPass* renderPass, VertexDescription* vertexDescription, Shader* shader);

		// This must explictly be called
		// The constructor sets default values and to make modifications to the pipeline they should be made between the constructor and Create()
		void Create();

		/*
			Creates the descriptor set layout and pipeline layout objects	
			\note Must be called before calling Create()
		*/
		void CreateLayouts(Device* device);

		void AddUniformBuffer(uint32_t descriptorSet, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);
		void AddStorageBuffer(uint32_t descriptorSet, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);
		void AddCombinedImageSampler(uint32_t descriptorSet, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);
		void AddPushConstantRange(uint32_t size, VkShaderStageFlags shaderStage, uint32_t offset = 0);

		VkDescriptorSetLayout GetDescriptorSetLayout(uint32_t descriptorSet);
		VkPipelineLayout GetPipelineLayout();

		VkPipelineRasterizationStateCreateInfo mRasterizationState = {};
		VkPipelineInputAssemblyStateCreateInfo mInputAssemblyState = {};
		VkPipelineColorBlendAttachmentState mBlendAttachmentState = {};
	private:
		void AddDescriptor(VkDescriptorType descriptorType, uint32_t descriptorSet, uint32_t binding, uint32_t descriptorCount, VkShaderStageFlags stageFlags);

		VkPipelineLayout mPipelineLayout;
		//PipelineLayout* mPipelineLayout = nullptr;
		RenderPass* mRenderPass = nullptr;
		VertexDescription* mVertexDescription = nullptr;
		Shader* mShader = nullptr;

		std::map<uint32_t, DescriptorSetLayout> mDescriptorSetLayouts;
		std::vector<VkPushConstantRange> mPushConstantRanges;
	};
}
