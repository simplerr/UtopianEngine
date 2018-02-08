#pragma once

#include <array>
#include "Handle.h"
#include "vulkan/VulkanInclude.h"

namespace Utopian::Vk
{
	class Pipeline : public Handle<VkPipeline>
	{
	public:
		Pipeline(Device* device, PipelineLayout* pipelineLayout, RenderPass* renderPass, VertexDescription* vertexDescription, Shader* shader);

		// This must explicitly be called
		// The constructor sets default values and to make modifications to the pipeline they should be made between the constructor and Create()
		void Create();

		VkPipelineRasterizationStateCreateInfo mRasterizationState = {};
		VkPipelineInputAssemblyStateCreateInfo mInputAssemblyState = {};
		VkPipelineColorBlendAttachmentState mBlendAttachmentState = {};
	private:
		PipelineLayout* mPipelineLayout = nullptr;
		RenderPass* mRenderPass = nullptr;
		VertexDescription* mVertexDescription = nullptr;
		Shader* mShader = nullptr;
	};
}
