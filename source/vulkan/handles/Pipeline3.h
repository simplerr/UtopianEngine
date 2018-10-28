#pragma once

#include <array>
#include <vector>
#include <map>
#include "Handle.h"
#include "vulkan/VulkanInclude.h"
#include "vulkan/VertexDescription.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/DescriptorSet.h"
#include "utility/Common.h"

namespace Utopian::Vk
{
	class Pipeline3 : public Handle<VkPipeline>
	{
	public:
		Pipeline3(Device* device, RenderPass* renderPass, const VertexDescription& vertexDescription, SharedPtr<Shader> shader);
		Pipeline3();

		// This must explictly be called
		// The constructor sets default values and to make modifications to the pipeline they should be made between the constructor and Create()
		void Create();

		void Init(Device* device, RenderPass* renderPass, const VertexDescription& vertexDescription, SharedPtr<Shader> shader);

		void BindUniformBuffer(std::string name, VkDescriptorBufferInfo* bufferInfo);
		void BindStorageBuffer(std::string name, VkDescriptorBufferInfo* bufferInfo);
		void BindCombinedImage(std::string name, VkDescriptorImageInfo* imageInfo);
		void BindCombinedImage(std::string name, Image* image, Sampler* sampler);

		void BindDescriptorSets(CommandBuffer* commandBuffer);

		PipelineInterface* GetPipelineInterface();

		SharedPtr<Shader> GetShader();

		VkPipelineRasterizationStateCreateInfo mRasterizationState = {};
		VkPipelineInputAssemblyStateCreateInfo mInputAssemblyState = {};
		VkPipelineDepthStencilStateCreateInfo mDepthStencilState = {};
		std::vector<VkPipelineColorBlendAttachmentState> mBlendAttachmentState;
	private:
		void CreatePipelineInterface(const SharedPtr<Shader>& shader, Device* device);
		void InitDefaultValues();

		RenderPass* mRenderPass = nullptr;
		SharedPtr<Shader> mShader;
		VertexDescription mVertexDescription;
		PipelineInterface mPipelineInterface;
		std::vector<DescriptorSet> mDescriptorSets;
		std::vector<VkDescriptorSet> mVkDescriptorSets;
		DescriptorPool mDescriptorPool;
		bool mInitialized;
	};
}
