#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include "Handle.h"

namespace VulkanLib
{
	class VertexDescription;
	class RenderPass;
	class PipelineLayout;
	class Device;
	class Shader;

	class Pipeline : public Handle<VkPipeline>
	{
	public:
		Pipeline(Device* device, PipelineLayout* pipelineLayout, RenderPass* renderPass, VertexDescription* vertexDescription, Shader* shader);

		// This must explictly be called
		// The constructor sets default values and to make modifications to the pipeline they should be made between the constructor and Create()
		void Create();

		VkPipelineRasterizationStateCreateInfo mRasterizationState = {};
	private:
		PipelineLayout* mPipelineLayout = nullptr;
		RenderPass* mRenderPass = nullptr;
		VertexDescription* mVertexDescription = nullptr;
		Shader* mShader = nullptr;
	};
}
