#include "core/renderer/SSAOJob.h"
#include "core/renderer/GBufferJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include <random>

namespace Utopian
{
	SSAOJob::SSAOJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		ssaoImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT);

		renderTarget = std::make_shared<Vk::RenderTarget>(device, width, height);
		renderTarget->AddColorAttachment(ssaoImage);
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		effect = Vk::gEffectManager().AddEffect<Vk::SSAOEffect>(device, renderTarget->GetRenderPass());
	}

	SSAOJob::~SSAOJob()
	{
	}

	void SSAOJob::Init(const std::vector<BaseJob*>& renderers)
	{
		GBufferJob* gbufferRenderer = static_cast<GBufferJob*>(renderers[JobGraph::GBUFFER_INDEX]);
		effect->BindGBuffer(gbufferRenderer->positionImage.get(),
			gbufferRenderer->normalViewImage.get(),
			gbufferRenderer->albedoImage.get(),
			gbufferRenderer->renderTarget->GetSampler());

		CreateKernelSamples();
	}

	void SSAOJob::Render(const JobInput& jobInput)
	{
		effect->SetCameraData(jobInput.sceneInfo.viewMatrix, jobInput.sceneInfo.projectionMatrix, glm::vec4(jobInput.sceneInfo.eyePos, 1.0f));
		effect->SetSettings(jobInput.renderingSettings.ssaoRadius, jobInput.renderingSettings.ssaoBias);

		renderTarget->Begin();
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		// Todo: Should this be moved to the effect instead?
		commandBuffer->CmdBindPipeline(effect->GetPipeline());
		effect->BindDescriptorSets(commandBuffer);

		gRendererUtility().DrawFullscreenQuad(commandBuffer);

		renderTarget->End();
	}

	void SSAOJob::CreateKernelSamples()
	{
		// Kernel samples
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
		std::default_random_engine generator;
		for (unsigned int i = 0; i < 64; ++i)
		{
			glm::vec3 sample(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator)
			);
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);

			effect->cameraBlock.data.samples[i] = glm::vec4(sample, 0);
		}

		effect->UpdateMemory();
	}
}
