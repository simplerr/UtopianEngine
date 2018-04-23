#include "vulkan/GBufferEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/Renderer.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/ShaderManager.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/Vertex.h"

namespace Utopian::Vk
{
	GBufferEffect::GBufferEffect()
	{
	}

	void GBufferEffect::CreateDescriptorPool(Device* device)
	{
		mDescriptorPool = new Utopian::Vk::DescriptorPool(device);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
		mDescriptorPool->Create();
	}

	void GBufferEffect::CreateVertexDescription(Device* device)
	{
		// First tell Vulkan about how large each vertex is, the binding ID and the inputRate
		mVertexDescription = new Utopian::Vk::VertexDescription();
		mVertexDescription->AddBinding(BINDING_0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);

		// We need to tell Vulkan about the memory layout for each attribute
		// 5 attributes: position, normal, texture coordinates, tangent and color
		// See Vertex struct
		mVertexDescription->AddAttribute(BINDING_0, Vec3Attribute());	// Location 0 : Position
		mVertexDescription->AddAttribute(BINDING_0, Vec3Attribute());	// Location 1 : Color
		mVertexDescription->AddAttribute(BINDING_0, Vec3Attribute());	// Location 2 : Normal
		mVertexDescription->AddAttribute(BINDING_0, Vec2Attribute());	// Location 3 : Texture
		mVertexDescription->AddAttribute(BINDING_0, Vec4Attribute());	// Location 4 : Tangent
	}

	void GBufferEffect::CreatePipelineInterface(Device* device)
	{
		// Descriptor set 0
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_0, VK_SHADER_STAGE_VERTEX_BIT);
		mPipelineInterface.AddCombinedImageSampler(SET_1, BINDING_0, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddPushConstantRange(sizeof(PushConstantBlock), VK_SHADER_STAGE_VERTEX_BIT);
		mPipelineInterface.CreateLayouts(device);
	}

	void GBufferEffect::CreateDescriptorSets(Device* device)
	{
		per_frame_vs.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		mDescriptorSet0 = new Utopian::Vk::DescriptorSet(device, mPipelineInterface.GetDescriptorSetLayout(SET_0), mDescriptorPool);
		mDescriptorSet0->BindUniformBuffer(BINDING_0, &per_frame_vs.GetDescriptor());
		mDescriptorSet0->UpdateDescriptorSets();
	}

	void GBufferEffect::CreatePipeline(Renderer* renderer)
	{
		Shader* shader = renderer->mShaderManager->CreateShader("data/shaders/gbuffer/gbuffer.vert.spv", "data/shaders/gbuffer/gbuffer.frag.spv");

		Pipeline2*  pipeline = new Pipeline2(renderer->GetDevice(), mRenderPass, mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline->mDepthStencilState.depthTestEnable = VK_TRUE;
		pipeline->Create();
		mPipelines[Variation::NORMAL] = pipeline;
	}

	void GBufferEffect::UpdateMemory(Device* device)
	{
		per_frame_vs.UpdateMemory();
	}

	void GBufferEffect::SetRenderPass(RenderPass* renderPass)
	{
		mRenderPass = renderPass;
	}
}