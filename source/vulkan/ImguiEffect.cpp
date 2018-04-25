#include "vulkan/ImguiEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/Renderer.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/ShaderManager.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/handles/RenderPass.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/Vertex.h"
#include "imgui/imgui.h"

namespace Utopian::Vk
{
	ImguiEffect::ImguiEffect()
	{
	}

	void ImguiEffect::CreateDescriptorPool(Device* device)
	{
		mDescriptorPool = new DescriptorPool(device);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
		mDescriptorPool->Create();
	}

	void ImguiEffect::CreateVertexDescription(Device* device)
	{
		mVertexDescription.AddBinding(BINDING_0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX);

		mVertexDescription.AddAttribute(BINDING_0, Vec2Attribute());	// Location 0 : Position
		mVertexDescription.AddAttribute(BINDING_0, Vec2Attribute());	// Location 1 : Uv
		mVertexDescription.AddAttribute(BINDING_0, U32Attribute());		// Location 2 : Color
	}

	void ImguiEffect::CreatePipelineInterface(Device* device)
	{
		mPipelineInterface.AddCombinedImageSampler(SET_0, BINDING_0, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddPushConstantRange(sizeof(PushConstantBlock), VK_SHADER_STAGE_VERTEX_BIT);
		mPipelineInterface.CreateLayouts(device);
	}

	void ImguiEffect::CreateDescriptorSets(Device* device)
	{
		mDescriptorSet0 = new Utopian::Vk::DescriptorSet(device, mPipelineInterface.GetDescriptorSetLayout(SET_0), mDescriptorPool);
		mDescriptorSet0->BindCombinedImage(BINDING_0, &mTexture->GetTextureDescriptorInfo());
		mDescriptorSet0->UpdateDescriptorSets();
	}

	void ImguiEffect::CreatePipeline(Renderer* renderer)
	{
		// Render pass
		mRenderPass = new Utopian::Vk::RenderPass(renderer->GetDevice(), VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		//mRenderPass->attachments[RenderPassAttachment::COLOR_ATTACHMENT].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		mRenderPass->Create();

		Shader* shader = renderer->mShaderManager->CreateShader("data/shaders/imgui/uioverlay.vert.spv", "data/shaders/imgui/uioverlay.frag.spv");

		Pipeline2*  pipeline = new Pipeline2(renderer->GetDevice(), mRenderPass, mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);

		// Customize the pipeline with blending
		pipeline->mBlendAttachmentState[0].blendEnable = VK_TRUE;
		pipeline->mBlendAttachmentState[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		pipeline->mBlendAttachmentState[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		pipeline->mBlendAttachmentState[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipeline->mBlendAttachmentState[0].colorBlendOp = VK_BLEND_OP_ADD;
		pipeline->mBlendAttachmentState[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipeline->mBlendAttachmentState[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		pipeline->mBlendAttachmentState[0].alphaBlendOp = VK_BLEND_OP_ADD;

		// The front facing order is reversed in the Imgui vertex buffer
		pipeline->mRasterizationState.cullMode = VK_CULL_MODE_NONE;

		// No depth testing
		pipeline->mDepthStencilState.depthTestEnable = VK_FALSE;
		pipeline->mDepthStencilState.depthWriteEnable = VK_FALSE;

		pipeline->Create();
		mPipelines[0] = pipeline;
	}

	void ImguiEffect::UpdateMemory()
	{
		
	}
}