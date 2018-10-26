#pragma once

#include <array>
#include <vector>
#include <map>
#include "Handle.h"
#include "vulkan/VulkanInclude.h"
#include "vulkan/VertexDescription.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "utility/Common.h"

namespace Utopian::Vk
{
	class Pipeline3 : public Handle<VkPipeline>
	{
	public:
		Pipeline3(Device* device, RenderPass* renderPass, const VertexDescription& vertexDescription, SharedPtr<Shader> shader);

		// This must explictly be called
		// The constructor sets default values and to make modifications to the pipeline they should be made between the constructor and Create()
		void Create();

		void CreatePipelineInterface(const SharedPtr<Shader> shader, Device* device);
		PipelineInterface* GetPipelineInterface();

		VkPipelineRasterizationStateCreateInfo mRasterizationState = {};
		VkPipelineInputAssemblyStateCreateInfo mInputAssemblyState = {};
		VkPipelineDepthStencilStateCreateInfo mDepthStencilState = {};
		std::vector<VkPipelineColorBlendAttachmentState> mBlendAttachmentState;
	private:
		RenderPass* mRenderPass = nullptr;
		SharedPtr<Shader> mShader;
		VertexDescription mVertexDescription;
		PipelineInterface mPipelineInterface;
	};
}
