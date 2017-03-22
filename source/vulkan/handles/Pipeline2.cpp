#include "vulkan/ShaderManager.h"
#include "vulkan/Device.h"
#include "vulkan/VulkanDebug.h"
#include "vulkan/VertexDescription.h"
#include "Pipeline2.h"
#include "RenderPass.h"
#include "PipelineLayout.h"

namespace Vulkan
{
	Pipeline2::Pipeline2(Device* device, RenderPass* renderPass, VertexDescription* vertexDescription, Shader* shader)
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
		mBlendAttachmentState.colorWriteMask = 0xf;
		mBlendAttachmentState.blendEnable = VK_FALSE;			// Blending disabled
	}

	void Pipeline2::Create()
	{
		// The pipeline consists of many stages, where each stage can have different states
		// Creating a pipeline is simply defining the state for every stage (and some more...)
		// ...
		VkPipelineColorBlendStateCreateInfo colorBlendState = {};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &mBlendAttachmentState;

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

		// Depth and stencil state
		VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
		depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
		depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
		depthStencilState.stencilTestEnable = VK_FALSE;			// Stencil disabled
		depthStencilState.front = depthStencilState.back;

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
		pipelineCreateInfo.layout = mPipelineLayout;
		pipelineCreateInfo.renderPass = mRenderPass->GetVkHandle();
		pipelineCreateInfo.pVertexInputState = &mVertexDescription->GetInputState();		// From base - &vertices.inputState;
		pipelineCreateInfo.pInputAssemblyState = &mInputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &mRasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.stageCount = mShader->shaderStages.size();
		pipelineCreateInfo.pStages = mShader->shaderStages.data();

		// Create the colored pipeline	
		//rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		VulkanDebug::ErrorCheck(vkCreateGraphicsPipelines(GetDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mHandle));
	}

	void Pipeline2::CreateLayouts(Device* device)
	{
		// Create all VkDescriptorSetLayouts
		for (auto& setLayout : mDescriptorSetLayouts)
		{
			setLayout.second.Create(device);
		}

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		for (uint32_t i = 0; i < mDescriptorSetLayouts.size(); i++)
		{
			descriptorSetLayouts.push_back(mDescriptorSetLayouts[i].GetVkHandle());
		}

		// Create a VkPipelineLayout from all the VkDescriptorSetLayouts and Push Constant ranges
		VkPipelineLayoutCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createInfo.pNext = NULL;
		createInfo.setLayoutCount = descriptorSetLayouts.size();
		createInfo.pSetLayouts = descriptorSetLayouts.data();

		if (mPushConstantRanges.size() > 0)
		{
			createInfo.pushConstantRangeCount = mPushConstantRanges.size();
			createInfo.pPushConstantRanges = mPushConstantRanges.data();
		}

		VulkanDebug::ErrorCheck(vkCreatePipelineLayout(GetDevice(), &createInfo, nullptr, &mPipelineLayout));
	}

	void Pipeline2::AddUniformBuffer(uint32_t descriptorSet, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
	{
		AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorSet, binding, descriptorCount, stageFlags);
	}

	void Pipeline2::AddStorageBuffer(uint32_t descriptorSet, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
	{
		AddDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptorSet, binding, descriptorCount, stageFlags);
	}

	void Pipeline2::AddCombinedImageSampler(uint32_t descriptorSet, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
	{
		AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorSet, binding, descriptorCount, stageFlags);
	}

	void Pipeline2::AddPushConstantRange(uint32_t size, VkShaderStageFlags shaderStage, uint32_t offset)
	{
		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.stageFlags = shaderStage;
		pushConstantRange.size = size;
		pushConstantRange.offset = offset;

		mPushConstantRanges.push_back(pushConstantRange);
	}

	VkDescriptorSetLayout Pipeline2::GetDescriptorSetLayout(uint32_t descriptorSet)
	{
		return mDescriptorSetLayouts[descriptorSet].GetVkHandle();
	}

	VkPipelineLayout Pipeline2::GetPipelineLayout()
	{
		return mPipelineLayout;
	}

	void Pipeline2::AddDescriptor(VkDescriptorType descriptorType, uint32_t descriptorSet, uint32_t binding, uint32_t descriptorCount, VkShaderStageFlags stageFlags)
	{
		if (mDescriptorSetLayouts.find(descriptorSet) == mDescriptorSetLayouts.end())
		{
			mDescriptorSetLayouts[descriptorSet] = DescriptorSetLayout();
		}

		mDescriptorSetLayouts[descriptorSet].AddBinding(binding, descriptorType, descriptorCount, stageFlags);
	}
}