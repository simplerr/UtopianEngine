#include "vulkan/BlurEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/Renderer.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/ShaderManager.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/Vertex.h"
#include "core/renderer/Light.h"
#include "core/renderer/SceneJobs.h"

namespace Utopian::Vk
{
	BlurEffect::BlurEffect()
	{
	}

	void BlurEffect::CreateDescriptorPool(Device* device)
	{
		mDescriptorPool = new Utopian::Vk::DescriptorPool(device);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3);
		mDescriptorPool->Create();
	}

	void BlurEffect::CreateVertexDescription(Device* device)
	{
		mVertexDescription = ScreenQuadVertex::GetDescription();
	}

	void BlurEffect::CreatePipelineInterface(Device* device)
	{
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_0, VK_SHADER_STAGE_FRAGMENT_BIT);			// Eye ubo
		mPipelineInterface.AddCombinedImageSampler(SET_1, BINDING_0, VK_SHADER_STAGE_FRAGMENT_BIT);		// SSAO output
		mPipelineInterface.CreateLayouts(device);
	}

	void BlurEffect::CreateDescriptorSets(Device* device)
	{
		ubo.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		mDescriptorSet0 = new Utopian::Vk::DescriptorSet(device, mPipelineInterface.GetDescriptorSetLayout(SET_0), mDescriptorPool);
		mDescriptorSet0->BindUniformBuffer(BINDING_0, ubo.GetDescriptor());
		mDescriptorSet0->UpdateDescriptorSets();

		mDescriptorSet1 = new Utopian::Vk::DescriptorSet(device, mPipelineInterface.GetDescriptorSetLayout(SET_1), mDescriptorPool);
	}

	void BlurEffect::CreatePipeline(Renderer* renderer)
	{
		Shader* shader = renderer->mShaderManager->CreateShader("data/shaders/blur/blur.vert.spv", "data/shaders/blur/blur.frag.spv");

		Pipeline2*  pipeline = new Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline->mDepthStencilState.depthTestEnable = VK_TRUE;
		pipeline->Create();
		mPipelines[Variation::NORMAL] = pipeline;
	}

	void BlurEffect::UpdateMemory()
	{
		ubo.UpdateMemory();
	}

	void BlurEffect::BindSSAOOutput(Image* ssaoImage, Sampler* sampler)
	{
		mDescriptorSet1->BindCombinedImage(0, ssaoImage, sampler);
		mDescriptorSet1->UpdateDescriptorSets();
	}

	void BlurEffect::BindDescriptorSets(CommandBuffer* commandBuffer)
	{
		VkDescriptorSet descriptorSets[2] = { mDescriptorSet0->descriptorSet, mDescriptorSet1->descriptorSet };
		commandBuffer->CmdBindDescriptorSet(this, 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
	}

	void BlurEffect::SetRenderPass(RenderPass* renderPass)
	{
		mRenderPass = renderPass;
	}
	
	void BlurEffect::SetSettings(int blurRange)
	{
		ubo.data.blurRange = blurRange;
		UpdateMemory();
	}
}