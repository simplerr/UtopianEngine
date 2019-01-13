#include "vulkan/ScreenQuadEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/Renderer.h"
#include "vulkan/Vertex.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/PipelineInterface.h"

namespace Utopian::Vk
{
	ScreenQuadEffect::ScreenQuadEffect()
	{
	}

	void ScreenQuadEffect::CreateDescriptorPool(Device* device)
	{
		mDescriptorPool = new Utopian::Vk::DescriptorPool(device);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 20);
		mDescriptorPool->Create();
	}

	void ScreenQuadEffect::CreateVertexDescription(Device* device)
	{
		mVertexDescription = ScreenQuadVertex::GetDescription();
	}

	void ScreenQuadEffect::CreatePipelineInterface(Device* device)
	{
		// Descriptor set 0
		mPipelineInterface.AddCombinedImageSampler(SET_0, BINDING_0, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddPushConstantRange(sizeof(PushConstantBlock), VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.CreateLayouts(device);
	}

	void ScreenQuadEffect::CreateDescriptorSets(Device* device)
	{
		mDescriptorSet0 = new Utopian::Vk::DescriptorSet(device, mPipelineInterface.GetDescriptorSetLayout(SET_0), mDescriptorPool);
	}

	void ScreenQuadEffect::CreatePipeline(Renderer* renderer)
	{
		Shader* shader = gShaderFactory().CreateShader("data/shaders/screenquad/screenquad.vert.spv", "data/shaders/screenquad/screenquad.frag.spv");

		Pipeline2*  pipeline = new Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline->Create();
		mPipelines[0] = pipeline;
	}

	void ScreenQuadEffect::UpdateMemory()
	{
	}
}