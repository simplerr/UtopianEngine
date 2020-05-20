#include "core/renderer/SSAOJob.h"
#include "core/renderer/GBufferJob.h"
#include "core/renderer/GBufferTerrainJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "utility/math/Helpers.h"
#include <random>

namespace Utopian
{
	SSAOJob::SSAOJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		ssaoImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT);

		renderTarget = std::make_shared<Vk::RenderTarget>(device, width, height);
		renderTarget->AddWriteOnlyColorAttachment(ssaoImage);
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		mEffect = Vk::gEffectManager().AddEffect<Vk::SSAOEffect>(device, renderTarget->GetRenderPass());
	}

	SSAOJob::~SSAOJob()
	{
	}

	void SSAOJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		GBufferTerrainJob* gbufferTerrainJob = static_cast<GBufferTerrainJob*>(jobs[JobGraph::GBUFFER_TERRAIN_INDEX]);
		GBufferJob* gbufferJob = static_cast<GBufferJob*>(jobs[JobGraph::GBUFFER_INDEX]);

		Vk::Sampler* sampler = gbufferTerrainJob->renderTarget->GetSampler();
		mEffect->BindCombinedImage("positionSampler", *gbuffer.positionImage, *sampler);
		mEffect->BindCombinedImage("normalSampler", *gbuffer.normalViewImage, *sampler);
		mEffect->BindCombinedImage("albedoSampler", *gbuffer.albedoImage, *sampler);

		CreateKernelSamples();
	}

	void SSAOJob::Render(const JobInput& jobInput)
	{
		mEffect->SetCameraData(jobInput.sceneInfo.viewMatrix, jobInput.sceneInfo.projectionMatrix, glm::vec4(jobInput.sceneInfo.eyePos, 1.0f));
		mEffect->SetSettings(jobInput.renderingSettings.ssaoRadius, jobInput.renderingSettings.ssaoBias);

		renderTarget->Begin("SSAO pass", glm::vec4(0.0, 1.0, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		if (IsEnabled())
		{
			commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
			commandBuffer->CmdBindDescriptorSets(mEffect);
			gRendererUtility().DrawFullscreenQuad(commandBuffer);
		}

		renderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
	}

	void SSAOJob::CreateKernelSamples()
	{
		// Kernel samples
		for (unsigned int i = 0; i < 64; ++i)
		{
			glm::vec3 sample(
				Math::GetRandom(0.0f, 1.0f) * 2.0 - 1.0,
				Math::GetRandom(0.0f, 1.0f) * 2.0 - 1.0,
				Math::GetRandom(0.0f, 1.0f)
			);
			sample = glm::normalize(sample);
			sample *= Math::GetRandom(0.0f, 1.0f);

			mEffect->cameraBlock.data.samples[i] = glm::vec4(sample, 0);
		}

		mEffect->UpdateMemory();
	}
}
