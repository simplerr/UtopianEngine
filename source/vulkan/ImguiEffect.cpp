#include "vulkan/ImguiEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/handles/RenderPass.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/Vertex.h"
#include "vulkan/VertexDescription.h"
#include "imgui/imgui.h"

namespace Utopian::Vk
{
	ImguiEffect::ImguiEffect(Device* device, RenderPass* renderPass)
		: Effect(device, renderPass)
	{
		ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/imgui/uioverlay.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/imgui/uioverlay.frag";
		SetShaderCreateInfo(shaderCreateInfo);

		// Customize the pipeline with blending
		mPipeline->blendAttachmentState[0].blendEnable = VK_TRUE;
		mPipeline->blendAttachmentState[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		mPipeline->blendAttachmentState[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		mPipeline->blendAttachmentState[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		mPipeline->blendAttachmentState[0].colorBlendOp = VK_BLEND_OP_ADD;
		mPipeline->blendAttachmentState[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		mPipeline->blendAttachmentState[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		mPipeline->blendAttachmentState[0].alphaBlendOp = VK_BLEND_OP_ADD;

		// The front facing order is reversed in the Imgui vertex buffer
		mPipeline->rasterizationState.cullMode = VK_CULL_MODE_NONE;

		// No depth testing
		mPipeline->depthStencilState.depthTestEnable = VK_FALSE;
		mPipeline->depthStencilState.depthWriteEnable = VK_FALSE;

		// Need to override vertex input description from shader since there is some special
		// treatment of U32 -> vec4 in ImGui
		mVertexDescription = std::make_shared<VertexDescription>();
		mVertexDescription->AddBinding(BINDING_0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX);
		mVertexDescription->AddAttribute(BINDING_0, Vec2Attribute());	// Location 0 : Position
		mVertexDescription->AddAttribute(BINDING_0, Vec2Attribute());	// Location 1 : Uv
		mVertexDescription->AddAttribute(BINDING_0, U32Attribute());	// Location 2 : Color

		mPipeline->OverrideVertexInput(mVertexDescription);

		CreatePipeline();
	}

	void ImguiEffect::UpdateMemory()
	{
		
	}
}