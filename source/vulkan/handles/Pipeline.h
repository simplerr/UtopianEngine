#pragma once

#include "Handle.h"
#include "vulkan/VulkanInclude.h"
#include "utility/Common.h"
#include <vector>

namespace Utopian::Vk
{
	class Pipeline : public Handle<VkPipeline>
	{
	public:
		Pipeline(Device* device, RenderPass* renderPass);

		void Create(Shader* shader, PipelineInterface* pipelineInterface);

		VkPipelineRasterizationStateCreateInfo rasterizationState = {};
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
		VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentState;
	private:
		void InitDefaultValues(RenderPass* renderPass);

		RenderPass* mRenderPass;	
	};
}
