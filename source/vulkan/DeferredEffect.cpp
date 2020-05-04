#include "vulkan/DeferredEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/Vertex.h"
#include "core/renderer/Light.h"
#include "core/renderer/RenderSettings.h"

namespace Utopian::Vk
{
	DeferredEffect::DeferredEffect(Device* device, RenderPass* renderPass)
		: Effect(device, renderPass)
	{
		ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/deferred/deferred.frag";
		SetShaderCreateInfo(shaderCreateInfo);

		mPipeline->rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		mPipeline->depthStencilState.depthTestEnable = VK_TRUE;

		// Vertices generated in fullscreen.vert are in clockwise order
		mPipeline->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mPipeline->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		CreatePipeline();

		eyeBlock.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		light_ubo.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		settings_ubo.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		cascade_ubo.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		BindUniformBuffer("UBO_eyePos", &eyeBlock);
		BindUniformBuffer("UBO_lights", &light_ubo);
		BindUniformBuffer("UBO_settings", &settings_ubo);
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

	void DeferredEffect::SetSettingsData(const RenderingSettings& renderingSettings)
	{
		// Todo:
		settings_ubo.data.fogColor = renderingSettings.fogColor;
		settings_ubo.data.fogStart = renderingSettings.fogStart;
		settings_ubo.data.fogDistance = renderingSettings.fogDistance;
		settings_ubo.data.cascadeColorDebug = renderingSettings.cascadeColorDebug;
		settings_ubo.UpdateMemory();
	}
}