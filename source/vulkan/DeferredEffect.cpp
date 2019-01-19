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
		: Effect(device, renderPass, "data/shaders/common/fullscreen.vert", "data/shaders/deferred/deferred.frag")
	{
		mPipeline->rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		mPipeline->depthStencilState.depthTestEnable = VK_TRUE;

		// Vertices generated in fullscreen.vert are in clockwise order
		mPipeline->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mPipeline->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		CreatePipeline();

		eyeBlock.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		light_ubo.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		settings_ubo.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		lightTransform.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		cascade_ubo.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		BindUniformBuffer("UBO_eyePos", &eyeBlock);
		BindUniformBuffer("UBO_lights", &light_ubo);
		BindUniformBuffer("UBO_settings", &settings_ubo);
		BindUniformBuffer("UBO_lightTransform", &lightTransform);
		BindUniformBuffer("UBO_cascades", &cascade_ubo);
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

	void DeferredEffect::SetLightTransform(glm::mat4 viewProjection)
	{
		lightTransform.data.viewProjection = viewProjection;
		lightTransform.UpdateMemory();
	}

	void DeferredEffect::SetSettingsData(const RenderingSettings& renderingSettings)
	{
		// Todo:
		settings_ubo.data.fogColor = renderingSettings.fogColor;
		settings_ubo.data.fogStart = renderingSettings.fogStart;
		settings_ubo.data.fogDistance = renderingSettings.fogDistance;
		settings_ubo.data.shadowSampleSize = renderingSettings.shadowSampleSize;
		settings_ubo.data.cascadeColorDebug = renderingSettings.cascadeColorDebug;
		settings_ubo.data.shadowsEnabled = renderingSettings.shadowsEnabled;
		settings_ubo.data.ssaoEnabled = renderingSettings.ssaoEnabled;
		settings_ubo.UpdateMemory();
	}
}