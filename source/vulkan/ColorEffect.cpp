#include "vulkan/ColorEffect.h"
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
	ColorEffect::ColorEffect()
	{
	}

	void ColorEffect::CreateDescriptorPool(Device* device)
	{
		mDescriptorPool = new Utopian::Vk::DescriptorPool(device);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
		mDescriptorPool->Create();
	}

	void ColorEffect::CreateVertexDescription(Device* device)
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

	void ColorEffect::CreatePipelineInterface(Device* device)
	{
		// Descriptor set 0
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_0, VK_SHADER_STAGE_VERTEX_BIT);
		mPipelineInterface.AddPushConstantRange(sizeof(PushConstantBlock), VK_SHADER_STAGE_VERTEX_BIT);
		mPipelineInterface.CreateLayouts(device);
	}

	void ColorEffect::CreateDescriptorSets(Device* device)
	{
		per_frame_vs.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		mDescriptorSet0 = new Utopian::Vk::DescriptorSet(device, mPipelineInterface.GetDescriptorSetLayout(SET_0), mDescriptorPool);
		mDescriptorSet0->BindUniformBuffer(BINDING_0, &per_frame_vs.GetDescriptor());
		mDescriptorSet0->UpdateDescriptorSets();
	}

	void ColorEffect::CreatePipeline(Renderer* renderer)
	{
		Shader* shader = renderer->mShaderManager->CreateShader("data/shaders/color/color.vert.spv", "data/shaders/color/color.frag.spv");

		Pipeline2*  pipeline = new Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		pipeline->mDepthStencilState.depthTestEnable = VK_FALSE;
		pipeline->Create();
		mPipelines[0] = pipeline;
	}

	void ColorEffect::UpdateMemory(Device* device)
	{
		per_frame_vs.UpdateMemory();
	}
}