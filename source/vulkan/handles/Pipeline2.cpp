#include "vulkan/ShaderFactory.h"
#include "vulkan/Device.h"
#include "vulkan/VulkanDebug.h"
#include "vulkan/VertexDescription.h"
#include "vulkan/PipelineInterface.h"
#include "Pipeline2.h"
#include "RenderPass.h"
#include "PipelineLayout.h"

namespace Utopian::Vk
{
	Pipeline2::Pipeline2(Device* device, RenderPass* renderPass, const VertexDescription& vertexDescription, Shader* shader)
		: Handle(device, vkDestroyPipeline)
	{
		mRenderPass = renderPass;
		mVertexDescription = vertexDescription;
		mShader = shader;

		// Rasterization state default values
		mRasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		mRasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
		mRasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		mRasterizationState.depthClampEnable = VK_FALSE;
		mRasterizationState.rasterizerDiscardEnable = VK_FALSE;
		mRasterizationState.depthBiasEnable = VK_FALSE;
		mRasterizationState.lineWidth = 1.0f;

		// Input assembly state
		mInputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mInputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		// Color blend state
		for (uint32_t i = 0; i < renderPass->colorReferences.size(); i++) {
			VkPipelineColorBlendAttachmentState colorBlend = {};
			colorBlend.colorWriteMask = 0xf;
			colorBlend.blendEnable = VK_FALSE;					// Blending disabled
			mBlendAttachmentState.push_back(colorBlend);
		}
		//mBlendAttachmentState.colorWriteMask = 0xf;
		//mBlendAttachmentState.blendEnable = VK_FALSE;			// Blending disabled

		// Depth and stencil state
		mDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		mDepthStencilState.depthTestEnable = VK_TRUE;
		mDepthStencilState.depthWriteEnable = VK_TRUE;
		mDepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		mDepthStencilState.depthBoundsTestEnable = VK_FALSE;
		mDepthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
		mDepthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
		mDepthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
		mDepthStencilState.stencilTestEnable = VK_FALSE;			// Stencil disabled
		mDepthStencilState.front = mDepthStencilState.back;
	}

	void Pipeline2::Create()
	{
		// The pipeline consists of many stages, where each stage can have different states
		// Creating a pipeline is simply defining the state for every stage (and some more...)
		// ...
		VkPipelineColorBlendStateCreateInfo colorBlendState = {};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.attachmentCount = mBlendAttachmentState.size();
		colorBlendState.pAttachments = mBlendAttachmentState.data();

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

																			// Load shader
																			//std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
																			//shaderStages[0] = LoadShader("data/shaders/phong/phong.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
																			//shaderStages[1] = LoadShader("data/shaders/phong/phong.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

																			// Assign all the states to the pipeline
																			// The states will be static and can't be changed after the pipeline is created
		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.layout = mPipelineInterface->GetPipelineLayout();
		pipelineCreateInfo.renderPass = mRenderPass->GetVkHandle();
		pipelineCreateInfo.pVertexInputState = &mVertexDescription.GetInputState();		// From base - &vertices.inputState;
		pipelineCreateInfo.pInputAssemblyState = &mInputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &mRasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.pDepthStencilState = &mDepthStencilState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.stageCount = mShader->shaderStages.size();
		pipelineCreateInfo.pStages = mShader->shaderStages.data();

		// Create the colored pipeline	
		//rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		VulkanDebug::ErrorCheck(vkCreateGraphicsPipelines(GetDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mHandle));
	}

	void Pipeline2::SetPipelineInterface(PipelineInterface* pipelineInterface)
	{
		mPipelineInterface = pipelineInterface;
	}

	VkPipelineLayout Pipeline2::GetPipelineLayout()
	{
		return mPipelineInterface->GetPipelineLayout();
	}
}