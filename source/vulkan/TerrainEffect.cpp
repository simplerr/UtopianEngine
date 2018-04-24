#include "vulkan/TerrainEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/Renderer.h"
#include "vulkan/Vertex.h"
#include "vulkan/ShaderManager.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/PipelineInterface.h"

namespace Utopian::Vk
{
	TerrainEffect::TerrainEffect()
	{
		SetPipeline(PipelineType2::WIREFRAME);
	}

	void TerrainEffect::CreateDescriptorPool(Device* device)
	{
		mDescriptorPool = new Utopian::Vk::DescriptorPool(device);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
		mDescriptorPool->Create();
	}

	void TerrainEffect::CreateVertexDescription(Device* device)
	{
		mVertexDescription = TerrainVertex::GetDescription();
	}

	void TerrainEffect::CreatePipelineInterface(Device* device)
	{
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_0, VK_SHADER_STAGE_VERTEX_BIT);						// per_frame_vs UBO
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_1, VK_SHADER_STAGE_FRAGMENT_BIT);					// per_frame_ps UBO
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_2, VK_SHADER_STAGE_FRAGMENT_BIT);					// per_frame_ps UBO
		mPipelineInterface.AddUniformBuffer(SET_1, BINDING_0, VK_SHADER_STAGE_FRAGMENT_BIT);					// per_frame_ps UBO
		mPipelineInterface.AddCombinedImageSampler(SET_1, BINDING_1, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddPushConstantRange(sizeof(PushConstantBasicBlock), VK_SHADER_STAGE_VERTEX_BIT);	// pushConsts
		mPipelineInterface.CreateLayouts(device);
	}

	void TerrainEffect::CreateDescriptorSets(Device* device)
	{
		
	}

	void TerrainEffect::CreatePipeline(Renderer* renderer)
	{
		Utopian::Vk::Shader* shader = renderer->mShaderManager->CreateShader("data/shaders/terrain/terrain.vert.spv", "data/shaders/terrain/terrain.frag.spv");

		Pipeline2* pipeline = new Utopian::Vk::Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		pipeline->Create();
		mPipelines[PipelineType2::WIREFRAME] = pipeline;

		Pipeline2* pipeline1 = new Utopian::Vk::Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);
		pipeline1->SetPipelineInterface(&mPipelineInterface);
		pipeline1->mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline1->Create();
		mPipelines[PipelineType2::SOLID] = pipeline1;
	}

	void TerrainEffect::UpdateMemory(Device* device)
	{
	}
}