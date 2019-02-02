#include "core/renderer/DeferredJob.h"
#include "core/renderer/GBufferJob.h"
#include "core/renderer/BlurJob.h"
#include "core/renderer/ShadowJob.h"
#include "core/renderer/CommonJobIncludes.h"

namespace Utopian
{
	DeferredJob::DeferredJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		renderTarget = std::make_shared<Vk::BasicRenderTarget>(device, width, height, VK_FORMAT_R8G8B8A8_UNORM);
		effect = Vk::gEffectManager().AddEffect<Vk::DeferredEffect>(device, renderTarget->GetRenderPass());

		mScreenQuad = gScreenQuadUi().AddQuad(0u, 0u, width, height, renderTarget->GetColorImage(), renderTarget->GetSampler(), 1u);

		// Create sampler that returns 1.0 when sampling outside the depth image
		depthSampler = std::make_shared<Vk::Sampler>(device, false);
		depthSampler->createInfo.anisotropyEnable = VK_FALSE; // Anistropy filter causes artifacts at the edge between cascades
		depthSampler->createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		depthSampler->createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		depthSampler->createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		depthSampler->createInfo.borderColor = VkBorderColor::VK_BORDER_COLOR_INT_OPAQUE_WHITE;
		depthSampler->Create();
	}

	DeferredJob::~DeferredJob()
	{
	}

	void DeferredJob::Init(const std::vector<BaseJob*>& jobs)
	{
		GBufferJob* gbufferJob = static_cast<GBufferJob*>(jobs[JobGraph::GBUFFER_INDEX]);
		BlurJob* blurJob = static_cast<BlurJob*>(jobs[JobGraph::BLUR_INDEX]);
		ShadowJob* shadowJob = static_cast<ShadowJob*>(jobs[JobGraph::SHADOW_INDEX]);
		effect->BindImages(gbufferJob->positionImage.get(),
			gbufferJob->normalImage.get(),
			gbufferJob->albedoImage.get(),
			blurJob->blurImage.get(),
			gbufferJob->renderTarget->GetSampler());

		effect->BindCombinedImage("shadowSampler", shadowJob->depthColorImage.get(), depthSampler.get());
	}

	void DeferredJob::Render(const JobInput& jobInput)
	{
		effect->SetSettingsData(jobInput.renderingSettings);
		effect->SetEyePos(glm::vec4(jobInput.sceneInfo.eyePos, 1.0f));
		effect->SetLightArray(jobInput.sceneInfo.lights);

		// Note: Todo: Temporary
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			effect->cascade_ubo.data.cascadeSplits[i] = jobInput.sceneInfo.cascades[i].splitDepth;
			effect->cascade_ubo.data.cascadeViewProjMat[i] = jobInput.sceneInfo.cascades[i].viewProjMatrix;
		}

		// Note: This should probably be moved. We need the fragment position in view space
		// when comparing it's Z value to find out which shadow map cascade it should sample from.
		effect->cascade_ubo.data.cameraViewMat = jobInput.sceneInfo.viewMatrix;
		effect->cascade_ubo.UpdateMemory();

		// End of temporary

		renderTarget->Begin("Deferred pass", glm::vec4(0.0, 1.0, 1.0, 1.0));
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		// Todo: Should this be moved to the effect instead?
		commandBuffer->CmdBindPipeline(effect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(effect);

		gRendererUtility().DrawFullscreenQuad(commandBuffer);

		renderTarget->End();
	}
}
