#include "core/renderer/jobs/DeferredJob.h"
#include "core/renderer/jobs/BlurJob.h"
#include "core/renderer/jobs/ShadowJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "vulkan/Effect.h"

namespace Utopian
{
	DeferredJob::DeferredJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		renderTarget = std::make_shared<Vk::BasicRenderTarget>(device, width, height, VK_FORMAT_R32G32B32A32_SFLOAT);

		Vk::EffectCreateInfo effectDesc;
		effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/deferred/deferred.frag";

		mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(device, renderTarget->GetRenderPass(), effectDesc);

		// Create sampler that returns 1.0 when sampling outside the depth image
		mDepthSampler = std::make_shared<Vk::Sampler>(device, false);
		mDepthSampler->createInfo.anisotropyEnable = VK_FALSE; // Anistropic filter causes artifacts at the edge between cascades
		mDepthSampler->createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		mDepthSampler->createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		mDepthSampler->createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		mDepthSampler->createInfo.borderColor = VkBorderColor::VK_BORDER_COLOR_INT_OPAQUE_WHITE;
		mDepthSampler->Create();

		mSampler = std::make_shared<Vk::Sampler>(mDevice);

		//mScreenQuad = gScreenQuadUi().AddQuad(0u, 0u, width, height, renderTarget->GetColorImage().get(), renderTarget->GetSampler(), 1u);
	}

	DeferredJob::~DeferredJob()
	{
	}

	void DeferredJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		BlurJob* blurJob = static_cast<BlurJob*>(jobs[JobGraph::BLUR_INDEX]);
		ShadowJob* shadowJob = static_cast<ShadowJob*>(jobs[JobGraph::SHADOW_INDEX]);

		light_ubo.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		settings_ubo.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		cascade_ubo.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		mEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
		mEffect->BindUniformBuffer("UBO_lights", light_ubo);
		mEffect->BindUniformBuffer("UBO_settings", settings_ubo);
		mEffect->BindUniformBuffer("UBO_cascades", cascade_ubo);

		mEffect->BindCombinedImage("positionSampler", *gbuffer.positionImage, *mSampler);
		mEffect->BindCombinedImage("normalSampler", *gbuffer.normalImage, *mSampler);
		mEffect->BindCombinedImage("albedoSampler", *gbuffer.albedoImage, *mSampler);
		mEffect->BindCombinedImage("ssaoSampler", *blurJob->blurImage, *mSampler);
		mEffect->BindCombinedImage("shadowSampler", *shadowJob->depthColorImage, *mDepthSampler);
	}

	void DeferredJob::Render(const JobInput& jobInput)
	{
		// Settings data
		settings_ubo.data.fogColor = jobInput.renderingSettings.fogColor;
		settings_ubo.data.fogStart = jobInput.renderingSettings.fogStart;
		settings_ubo.data.fogDistance = jobInput.renderingSettings.fogDistance;
		settings_ubo.data.cascadeColorDebug = jobInput.renderingSettings.cascadeColorDebug;
		settings_ubo.UpdateMemory();

		// Light array
		light_ubo.lights.clear();
		for (auto& light : jobInput.sceneInfo.lights)
		{
			light_ubo.lights.push_back(light->GetLightData());
		}

		light_ubo.constants.numLights = light_ubo.lights.size();
		light_ubo.UpdateMemory();

		// Note: Todo: Temporary
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			cascade_ubo.data.cascadeSplits[i] = jobInput.sceneInfo.cascades[i].splitDepth;
			cascade_ubo.data.cascadeViewProjMat[i] = jobInput.sceneInfo.cascades[i].viewProjMatrix;
		}

		// Note: This should probably be moved. We need the fragment position in view space
		// when comparing it's Z value to find out which shadow map cascade it should sample from.
		cascade_ubo.data.cameraViewMat = jobInput.sceneInfo.viewMatrix;
		cascade_ubo.data.shadowSampleSize = jobInput.renderingSettings.shadowSampleSize;
		cascade_ubo.data.shadowsEnabled = jobInput.renderingSettings.shadowsEnabled;
		cascade_ubo.UpdateMemory();

		// End of temporary

		renderTarget->Begin("Deferred pass", glm::vec4(0.0, 1.0, 1.0, 1.0));
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		// Todo: Should this be moved to the effect instead?
		commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(mEffect);

		gRendererUtility().DrawFullscreenQuad(commandBuffer);

		renderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
	}
}
