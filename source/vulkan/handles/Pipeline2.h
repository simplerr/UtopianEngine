#pragma once

#include <array>
#include <vector>
#include <map>
#include "Handle.h"
#include "vulkan/VulkanInclude.h"
#include "vulkan/VertexDescription.h"
#include "vulkan/handles/DescriptorSetLayout.h"

namespace Utopian::Vk
{
	/** Todo: Note: Legacy. */
	class Pipeline2 : public Handle<VkPipeline>
	{
	public:
		Pipeline2(Device* device, RenderPass* renderPass, const VertexDescription& vertexDescription, Shader* shader);

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
		Shader* mShader = nullptr;
		PipelineInterface* mPipelineInterface;
		VertexDescription mVertexDescription;
	};
}
