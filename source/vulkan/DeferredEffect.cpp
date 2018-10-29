#include "vulkan/DeferredEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/Renderer.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/Vertex.h"
#include "core/renderer/Light.h"
#include "core/renderer/SceneJobs.h"

namespace Utopian::Vk
{
	DeferredEffect::DeferredEffect(Device* device, RenderPass* renderPass)
		: Effect(device, renderPass, "data/shaders/deferred/deferred.vert", "data/shaders/deferred/deferred.frag")
	{
		mPipeline->rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		mPipeline->depthStencilState.depthTestEnable = VK_TRUE;
		CreatePipeline();

		eyeBlock.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		light_ubo.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		fog_ubo.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		BindUniformBuffer("UBO_eyePos", &eyeBlock);
		BindUniformBuffer("UBO_lights", &light_ubo);
		BindUniformBuffer("UBO_fog", &fog_ubo);
	}

	void DeferredEffect::UpdateMemory()
	{
		eyeBlock.UpdateMemory();
	}

	void DeferredEffect::SetEyePos(glm::vec3 eyePos)
	{
		eyeBlock.data.eyePos = glm::vec4(eyePos, 1.0f);
		eyeBlock.UpdateMemory();
	}

	void DeferredEffect::BindImages(Image* positionImage, Image* normalImage, Image* albedoImage, Image* ssaoImage, Sampler* sampler)
	{
		BindCombinedImage("positionSampler", positionImage, sampler);
		BindCombinedImage("normalSampler", normalImage, sampler);
		BindCombinedImage("albedoSampler", albedoImage, sampler);
		BindCombinedImage("ssaoSampler", ssaoImage, sampler);
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
}