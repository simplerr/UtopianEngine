#include "vulkan/SSAOEffect.h"
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
	SSAOEffect::SSAOEffect()
	{
	}

	void SSAOEffect::CreateDescriptorPool(Device* device)
	{
		mDescriptorPool = new Utopian::Vk::DescriptorPool(device);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3);
		mDescriptorPool->Create();
	}

	void SSAOEffect::CreateVertexDescription(Device* device)
	{
		mVertexDescription = ScreenQuadVertex::GetDescription();
	}

	void SSAOEffect::CreatePipelineInterface(Device* device)
	{
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_0, VK_SHADER_STAGE_FRAGMENT_BIT);			// Eye ubo
		mPipelineInterface.AddUniformBuffer(SET_2, BINDING_0, VK_SHADER_STAGE_FRAGMENT_BIT);			// SSAO settings ubo
		mPipelineInterface.AddCombinedImageSampler(SET_1, BINDING_0, VK_SHADER_STAGE_FRAGMENT_BIT);		// Position 
		mPipelineInterface.AddCombinedImageSampler(SET_1, BINDING_1, VK_SHADER_STAGE_FRAGMENT_BIT);		// Normal
		mPipelineInterface.AddCombinedImageSampler(SET_1, BINDING_2, VK_SHADER_STAGE_FRAGMENT_BIT);		// Albedo
		mPipelineInterface.CreateLayouts(device);
	}

	void SSAOEffect::CreateDescriptorSets(Device* device)
	{
		ubo.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		ubo_settings.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		mDescriptorSet0 = new Utopian::Vk::DescriptorSet(device, mPipelineInterface.GetDescriptorSetLayout(SET_0), mDescriptorPool);
		mDescriptorSet0->BindUniformBuffer(BINDING_0, &ubo.GetDescriptor());
		mDescriptorSet0->UpdateDescriptorSets();

		mDescriptorSet1 = new Utopian::Vk::DescriptorSet(device, mPipelineInterface.GetDescriptorSetLayout(SET_1), mDescriptorPool);

		mDescriptorSet2 = new Utopian::Vk::DescriptorSet(device, mPipelineInterface.GetDescriptorSetLayout(SET_2), mDescriptorPool);
		mDescriptorSet2->BindUniformBuffer(BINDING_0, &ubo_settings.GetDescriptor());
		mDescriptorSet2->UpdateDescriptorSets();
	}

	void SSAOEffect::CreatePipeline(Renderer* renderer)
	{
		Shader* shader = renderer->mShaderManager->CreateShader("data/shaders/ssao/ssao.vert.spv", "data/shaders/ssao/ssao.frag.spv");

		Pipeline2*  pipeline = new Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline->mDepthStencilState.depthTestEnable = VK_TRUE;
		pipeline->Create();
		mPipelines[Variation::NORMAL] = pipeline;
	}

	void SSAOEffect::UpdateMemory()
	{
		ubo.UpdateMemory();
		ubo_settings.UpdateMemory();
	}

	void SSAOEffect::SetEyePos(glm::vec3 eyePos)
	{
		ubo.data.eyePos = glm::vec4(eyePos, 1.0f);
		ubo.UpdateMemory();
	}

	void SSAOEffect::BindGBuffer(Image* positionImage, Image* normalViewImage, Image* albedoImage, Sampler* sampler)
	{
		mDescriptorSet1->BindCombinedImage(0, positionImage, sampler);
		mDescriptorSet1->BindCombinedImage(1, normalViewImage, sampler);
		mDescriptorSet1->BindCombinedImage(2, albedoImage, sampler);
		mDescriptorSet1->UpdateDescriptorSets();
	}

	void SSAOEffect::BindDescriptorSets(CommandBuffer* commandBuffer)
	{
		VkDescriptorSet descriptorSets[3] = { mDescriptorSet0->descriptorSet, mDescriptorSet1->descriptorSet, mDescriptorSet2->descriptorSet };
		commandBuffer->CmdBindDescriptorSet(this, 3, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
	}

	void SSAOEffect::SetRenderPass(RenderPass* renderPass)
	{
		mRenderPass = renderPass;
	}
	
	void SSAOEffect::SetCameraData(glm::mat4 view, glm::mat4 projection)
	{
		ubo.data.view = view;
		ubo.data.projection = projection;
		UpdateMemory();
	}

	void SSAOEffect::SetSettings(float radius, float bias)
	{
		ubo_settings.data.radius = radius;
		ubo_settings.data.bias = bias;
		UpdateMemory();
	}
}