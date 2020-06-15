#include "vulkan/handles/Pipeline.h"
#include "vulkan/handles/RenderPass.h"
#include "vulkan/Debug.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/ShaderFactory.h"
#include "core/renderer/RendererUtility.h"

namespace Utopian::Vk
{
	PipelineDesc::PipelineDesc()
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
	}

	void PipelineDesc::AddTessellationState(uint32_t patchSize)
	{
		tessellationCreateInfo = std::make_shared<VkPipelineTessellationStateCreateInfo>();
		tessellationCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		tessellationCreateInfo->patchControlPoints = patchSize;
	}

	void PipelineDesc::OverrideVertexInput(SharedPtr<VertexDescription> vertexDescription)
	{
		overriddenVertexDescription = vertexDescription;
	}

	Pipeline::Pipeline(const PipelineDesc& pipelineDesc, Device* device, RenderPass* renderPass)
		: Handle(device, vkDestroyPipeline)
	{
		mRenderPass = renderPass;
		mCreated = false;
		mPipelineDesc = pipelineDesc;
	}

	void Pipeline::Create(Shader* shader, PipelineInterface* pipelineInterface)
	{
		// The pipeline consists of many stages, where each stage can have different states
		// Creating a pipeline is simply defining the state for every stage (and some more...)
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentState;
      for (uint32_t i = 0; i < mRenderPass->colorReferences.size(); i++) {
         VkPipelineColorBlendAttachmentState colorBlend = {};
         colorBlend.colorWriteMask = 0xf;
         colorBlend.blendEnable = VK_FALSE;
         blendAttachmentState.push_back(colorBlend);
      }

      if (mPipelineDesc.blendingType == BlendingType::BLENDING_ADDITIVE)
         SetAdditiveBlending(blendAttachmentState[0]);
      else if (mPipelineDesc.blendingType == BlendingType::BLENDING_ALPHA)
         SetAlphaBlending(blendAttachmentState[0]);

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

		if (mPipelineDesc.overriddenVertexDescription != nullptr)
			pipelineCreateInfo.pVertexInputState = &mPipelineDesc.overriddenVertexDescription->GetInputState();
		else
			pipelineCreateInfo.pVertexInputState = &shader->GetVertexDescription()->GetInputState();

		pipelineCreateInfo.pInputAssemblyState = &mPipelineDesc.inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &mPipelineDesc.rasterizationState;
		pipelineCreateInfo.pDepthStencilState = &mPipelineDesc.depthStencilState;
		pipelineCreateInfo.pTessellationState = mPipelineDesc.tessellationCreateInfo.get();
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.stageCount = shader->shaderStages.size();
		pipelineCreateInfo.pStages = shader->shaderStages.data();

		// Create the pipeline
		Debug::ErrorCheck(vkCreateGraphicsPipelines(GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mHandle));

		mCreated = true;
	}

	bool Pipeline::IsCreated() const
	{
		return mCreated;
	}

   void Pipeline::SetAdditiveBlending(VkPipelineColorBlendAttachmentState& blendAttachmentState)
   {
      // Enable additive blending
      blendAttachmentState.blendEnable = VK_TRUE;
      blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
      blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
      blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
      blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
      blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
      blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
   }

   void Pipeline::SetAlphaBlending(VkPipelineColorBlendAttachmentState& blendAttachmentState)
   {
      blendAttachmentState.blendEnable = VK_TRUE;
      blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
      blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
      blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
      blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
   }
}