#include "vulkan/PhongEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/Renderer.h"
#include "vulkan/ShaderManager.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/Vertex.h"
#include "utility/Utility.h"

namespace Vulkan
{
	PhongEffect::PhongEffect()
	{
		SetPipeline(PipelineType2::BASIC);
	}

	void PhongEffect::CreateDescriptorPool(Device* device)
	{
		mDescriptorPool = new DescriptorPool(device);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_NUM_TEXTURES);
		mDescriptorPool->Create();
	}

	void PhongEffect::CreateVertexDescription(Device* device)
	{
		// First tell Vulkan about how large each vertex is, the binding ID and the inputRate
		mVertexDescription = new Vulkan::VertexDescription();
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

	void PhongEffect::CreatePipelineInterface(Device* device)
	{
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_0, VK_SHADER_STAGE_VERTEX_BIT);
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_1, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_2, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddCombinedImageSampler(SET_1, BINDING_0, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddPushConstantRange(sizeof(PushConstantBlock), VK_SHADER_STAGE_VERTEX_BIT);
		mPipelineInterface.CreateLayouts(device);
	}

	void PhongEffect::CreateDescriptorSets(Device* device)
	{
	}

	void PhongEffect::CreatePipeline(Renderer* renderer)
	{
		// Load shader
		// [TODO] Move this into Pipeline?
		Shader* shader = renderer->mShaderManager->CreateShader("data/shaders/phong/phong.vert.spv", "data/shaders/phong/phong.frag.spv");

		// Solid pipeline
		Pipeline2*  pipeline = new Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline->Create();
		mPipelines[PipelineType2::BASIC] = pipeline;

		// Wireframe pipeline
		pipeline = new Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		pipeline->Create();
		mPipelines[PipelineType2::WIREFRAME] = pipeline;

		// Test pipeline
		Shader* testShader = renderer->mShaderManager->CreateShader("data/shaders/test/test.vert.spv", "data/shaders/test/test.frag.spv");
		pipeline = new Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		pipeline->mRasterizationState.cullMode = VK_CULL_MODE_NONE;
		pipeline->Create();
		mPipelines[PipelineType2::TEST] = pipeline;

		pipeline = new Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline->mRasterizationState.cullMode = VK_CULL_MODE_NONE;
		// TODO: Disable depth test
		pipeline->Create();
		mPipelines[PipelineType2::DEBUG] = pipeline;
	}

	void PhongEffect::UpdateMemory(Device* device)
	{
	}
}