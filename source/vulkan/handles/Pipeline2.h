#pragma once

#include <array>
#include <vector>
#include <map>
#include "Handle.h"
#include "vulkan/VulkanInclude.h"
#include "vulkan/handles/DescriptorSetLayout.h"

/* Defines to improve readability */
#define SET_0 0
#define SET_1 1
#define SET_2 2
#define SET_3 3
#define SET_4 4

#define BINDING_0 0
#define BINDING_1 1
#define BINDING_2 2
#define BINDING_3 3
#define BINDING_4 4

namespace Utopian::Vk
{
	class Pipeline2 : public Handle<VkPipeline>
	{
	public:
		Pipeline2(Device* device, RenderPass* renderPass, VertexDescription* vertexDescription, Shader* shader);

		// This must explictly be called
		// The constructor sets default values and to make modifications to the pipeline they should be made between the constructor and Create()
		void Create();

		void SetPipelineInterface(PipelineInterface* pipelineInterface);

		VkPipelineLayout GetPipelineLayout();

		VkPipelineRasterizationStateCreateInfo mRasterizationState = {};
		VkPipelineInputAssemblyStateCreateInfo mInputAssemblyState = {};
		VkPipelineDepthStencilStateCreateInfo mDepthStencilState = {};
		std::vector<VkPipelineColorBlendAttachmentState> mBlendAttachmentState;
	private:
		RenderPass* mRenderPass = nullptr;
		VertexDescription* mVertexDescription = nullptr;
		Shader* mShader = nullptr;
		PipelineInterface* mPipelineInterface;
	};
}
