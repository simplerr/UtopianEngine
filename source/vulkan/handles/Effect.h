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
		Effect(Device* device, RenderPass* renderPass, std::string vertexShader, std::string fragmentShader, std::string geometryShader = "NONE");

		void RecompileShader();
		// This must explictly be called
		// The constructor sets default values and to make modifications to the pipeline they should be made between the constructor and Create()
		void CreatePipeline();

		void BindUniformBuffer(std::string name, VkDescriptorBufferInfo* bufferInfo);
		void BindStorageBuffer(std::string name, VkDescriptorBufferInfo* bufferInfo);
		void BindCombinedImage(std::string name, VkDescriptorImageInfo* imageInfo, uint32_t descriptorCount = 1);
		void BindCombinedImage(std::string name, Image* image, Sampler* sampler);
		void BindCombinedImage(std::string name, TextureArray* textureArray);

		void BindUniformBuffer(std::string name, ShaderBuffer* shaderBlock);

		void BindDescriptorSets(CommandBuffer* commandBuffer);

		// Note: This should only be used in rare cases
		DescriptorSet& GetDescriptorSet(uint32_t set);

		PipelineInterface* GetPipelineInterface();
		Pipeline* GetPipeline();

		SharedPtr<Shader> GetShader();
		std::string GetVertexShaderPath();
	protected:
		SharedPtr<Pipeline> mPipeline;
	private:
		void Init();
		void CreatePipelineInterface(const SharedPtr<Shader>& shader, Device* device);

		RenderPass* mRenderPass = nullptr;
		Device* mDevice = nullptr;
		SharedPtr<Shader> mShader;
		SharedPtr<PipelineInterface> mPipelineInterface;
		std::vector<DescriptorSet> mDescriptorSets;
		std::vector<VkDescriptorSet> mVkDescriptorSets;
		std::string mVertexShaderPath;
		std::string mFragmentShaderPath;
		std::string mGeometryShaderPath;
		SharedPtr<DescriptorPool> mDescriptorPool;
	};
}
