#include "vulkan/handles/Pipeline.h"
#include "vulkan/handles/RenderPass.h"
#include "vulkan/VulkanDebug.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/ShaderFactory.h"

namespace Utopian::Vk
{
	Pipeline::Pipeline(Device* device, RenderPass* renderPass)
		: Handle(device, vkDestroyPipeline)
	{
		mRenderPass = renderPass;
		mCreated = false;
		InitDefaultValues(mRenderPass);
	}

	void Pipeline::Create(Shader* shader, PipelineInterface* pipelineInterface)
	{
		// The pipeline consists of many stages, where each stage can have different states
		// Creating a pipeline is simply defining the state for every stage (and some more...)
		// ...
		VkPipelineColorBlendStateCreateInfo colorBlendState = {};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.attachmentCount = blendAttachmentState.size();
		colorBlendState.pAttachments = blendAttachmentState.data();

		// Viewport state
		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		// Dynamic state for the viewport so the pipeline don't have to be recreated when resizing the window
		VkPipelineDynamicStateCreateInfo dynamicState = {};
		std::vector<VkDynamicState> dynamicStateEnables;
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = dynamicStateEnables.size();

		// Multi sampling state
		VkPipelineMultisampleStateCreateInfo multisampleState = {};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.pSampleMask = NULL;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;		// Multi sampling not used

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.layout = pipelineInterface->GetPipelineLayout();
		pipelineCreateInfo.renderPass = mRenderPass->GetVkHandle();
		pipelineCreateInfo.pVertexInputState = &shader->GetVertexDescription()->GetInputState();
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.stageCount = shader->shaderStages.size();
		pipelineCreateInfo.pStages = shader->shaderStages.data();

		// Create the colored pipeline	
		VulkanDebug::ErrorCheck(vkCreateGraphicsPipelines(GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mHandle));

		mCreated = true;
	}

	bool Pipeline::IsCreated() const
	{
		return mCreated;
	}

	void Pipeline::InitDefaultValues(RenderPass* renderPass)
	{
		// Rasterization state default values
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.depthBiasEnable = VK_FALSE;
		rasterizationState.lineWidth = 1.0f;

		// Input assembly state
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		// Depth and stencil state
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
		depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
		depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
		depthStencilState.stencilTestEnable = VK_FALSE;
		depthStencilState.front = depthStencilState.back;

		// Color blend state
		for (uint32_t i = 0; i < renderPass->colorReferences.size(); i++) {
			VkPipelineColorBlendAttachmentState colorBlend = {};
			colorBlend.colorWriteMask = 0xf;
			colorBlend.blendEnable = VK_FALSE;
			blendAttachmentState.push_back(colorBlend);
		}
	}
}