#include "vulkan/DeferredEffect.h"
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
#include "core/renderer/SceneRenderers.h"

namespace Utopian::Vk
{
	DeferredEffect::DeferredEffect()
	{
	}

	void DeferredEffect::CreateDescriptorPool(Device* device)
	{
		mDescriptorPool = new Utopian::Vk::DescriptorPool(device);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3);
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
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_0, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_1, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_2, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddCombinedImageSampler(SET_1, BINDING_0, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddCombinedImageSampler(SET_1, BINDING_1, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddCombinedImageSampler(SET_1, BINDING_2, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.CreateLayouts(device);
	}

	void DeferredEffect::CreateDescriptorSets(Device* device)
	{
		eye_ubo.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		light_ubo.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		fog_ubo.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		mDescriptorSet0 = new Utopian::Vk::DescriptorSet(device, mPipelineInterface.GetDescriptorSetLayout(SET_0), mDescriptorPool);
		mDescriptorSet0->BindUniformBuffer(BINDING_0, &eye_ubo.GetDescriptor());
		mDescriptorSet0->BindUniformBuffer(BINDING_1, &light_ubo.GetDescriptor());
		mDescriptorSet0->BindUniformBuffer(BINDING_2, &fog_ubo.GetDescriptor());
		mDescriptorSet0->UpdateDescriptorSets();

		mDescriptorSet1 = new Utopian::Vk::DescriptorSet(device, mPipelineInterface.GetDescriptorSetLayout(SET_1), mDescriptorPool);
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

	void DeferredEffect::UpdateMemory()
	{
		eye_ubo.UpdateMemory();
	}

	void DeferredEffect::SetEyePos(glm::vec3 eyePos)
	{
		eye_ubo.data.eyePos = glm::vec4(eyePos, 1.0f);
		eye_ubo.UpdateMemory();
	}

	void DeferredEffect::BindGBuffer(Image* positionImage, Image* normalImage, Image* albedoImage, Sampler* sampler)
	{
		mDescriptorSet1->BindCombinedImage(0, positionImage, sampler);
		mDescriptorSet1->BindCombinedImage(1, normalImage, sampler);
		mDescriptorSet1->BindCombinedImage(2, albedoImage, sampler);
		mDescriptorSet1->UpdateDescriptorSets();
	}

	void DeferredEffect::SetLightArray(const std::vector<Light*>& lights)
	{
		light_ubo.lights.clear();
		for (auto& light : lights)
		{
			light_ubo.lights.push_back(light->GetLightData());
		}

		light_ubo.constants.numLights = light_ubo.lights.size();

		light_ubo.UpdateMemory();
	}

	void DeferredEffect::SetFogData(const RenderingSettings& renderingSettings)
	{
		// Todo:
		fog_ubo.data.fogColor = renderingSettings.fogColor;
		fog_ubo.data.fogStart = renderingSettings.fogStart;
		fog_ubo.data.fogDistance = renderingSettings.fogDistance;
		fog_ubo.UpdateMemory();
	}

	void DeferredEffect::BindDescriptorSets(CommandBuffer* commandBuffer)
	{
		VkDescriptorSet descriptorSets[2] = { mDescriptorSet0->descriptorSet, mDescriptorSet1->descriptorSet };
		commandBuffer->CmdBindDescriptorSet(this, 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
	}
}