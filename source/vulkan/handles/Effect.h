#pragma once

#include <array>
#include <vector>
#include <map>
#include <string>
#include "vulkan/VulkanInclude.h"
#include "vulkan/VertexDescription.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/Pipeline.h"
#include "vulkan/PipelineInterface.h"
#include "utility/Common.h"

namespace Utopian::Vk
{
	class Effect
	{
	public:
		Effect(Device* device, RenderPass* renderPass, std::string vertexShader, std::string fragmentShader);

		// This must explictly be called
		// The constructor sets default values and to make modifications to the pipeline they should be made between the constructor and Create()
		void CreatePipeline();

		void BindUniformBuffer(std::string name, VkDescriptorBufferInfo* bufferInfo);
		void BindStorageBuffer(std::string name, VkDescriptorBufferInfo* bufferInfo);
		void BindCombinedImage(std::string name, VkDescriptorImageInfo* imageInfo);
		void BindCombinedImage(std::string name, Image* image, Sampler* sampler);

		void BindUniformBuffer(std::string name, ShaderBuffer* shaderBlock);

		void BindDescriptorSets(CommandBuffer* commandBuffer);

		// Note: This should only be used in rare cases
		DescriptorSet& GetDescriptorSet(uint32_t set);

		PipelineInterface* GetPipelineInterface();
		Pipeline* GetPipeline();

		SharedPtr<Shader> GetShader();
	protected:
		SharedPtr<Pipeline> mPipeline;
	private:
		void Init(Device* device, RenderPass* renderPass, std::string vertexShader, std::string fragmentShader);
		void CreatePipelineInterface(const SharedPtr<Shader>& shader, Device* device);

		RenderPass* mRenderPass = nullptr;
		SharedPtr<Shader> mShader;
		PipelineInterface mPipelineInterface;
		std::vector<DescriptorSet> mDescriptorSets;
		std::vector<VkDescriptorSet> mVkDescriptorSets;
		DescriptorPool mDescriptorPool;
	};
}
