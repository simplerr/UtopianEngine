#include "vulkan/DeferredEffect.h"
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
	DeferredEffect::DeferredEffect()
	{
	}

	void DeferredEffect::CreateDescriptorPool(Device* device)
	{
		mDescriptorPool = new Utopian::Vk::DescriptorPool(device);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3);
		mDescriptorPool->Create();
	}

	void DeferredEffect::CreateVertexDescription(Device* device)
	{
		mVertexDescription = ScreenQuadVertex::GetDescription();
	}

	void DeferredEffect::CreatePipelineInterface(Device* device)
	{
		// Descriptor set 0
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_1, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddCombinedImageSampler(SET_1, BINDING_0, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddCombinedImageSampler(SET_1, BINDING_1, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddCombinedImageSampler(SET_1, BINDING_2, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.CreateLayouts(device);
	}

	void DeferredEffect::CreateDescriptorSets(Device* device)
	{
		per_frame_ps.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		mDescriptorSet0 = new Utopian::Vk::DescriptorSet(device, mPipelineInterface.GetDescriptorSetLayout(SET_0), mDescriptorPool);
		mDescriptorSet0->BindUniformBuffer(BINDING_0, &per_frame_ps.GetDescriptor());
		mDescriptorSet0->UpdateDescriptorSets();

		// mDescriptorSet1 is initialized in SceneRenderer.cpp
	}

	void DeferredEffect::CreatePipeline(Renderer* renderer)
	{
		Shader* shader = renderer->mShaderManager->CreateShader("data/shaders/deferred/deferred.vert.spv", "data/shaders/deferred/deferred.frag.spv");

		Pipeline2*  pipeline = new Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline->mDepthStencilState.depthTestEnable = VK_TRUE;
		pipeline->Create();
		mPipelines[Variation::NORMAL] = pipeline;
	}

	void DeferredEffect::UpdateMemory(Device* device)
	{
		per_frame_ps.UpdateMemory();
	}
}