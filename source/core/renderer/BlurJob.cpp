#include "core/renderer/BlurJob.h"
#include "core/renderer/SSAOJob.h"
#include "core/renderer/CommonJobIncludes.h"

namespace Utopian
{
	BlurJob::BlurJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		blurImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT);

		renderTarget = std::make_shared<Vk::RenderTarget>(device, width, height);
		renderTarget->AddColorAttachment(blurImage);
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		effect = Vk::gEffectManager().AddEffect<Vk::BlurEffect>(device, renderTarget->GetRenderPass());

		const uint32_t size = 240;
		gScreenQuadUi().AddQuad(10, height - (size + 10), size, size, blurImage.get(), renderTarget->GetSampler());
	}

	BlurJob::~BlurJob()
	{
	}

	void BlurJob::Init(const std::vector<BaseJob*>& renderers)
	{
		SSAOJob* ssaoJob = static_cast<SSAOJob*>(renderers[RenderingManager::SSAO_INDEX]);
		effect->BindSSAOOutput(ssaoJob->ssaoImage.get(), ssaoJob->renderTarget->GetSampler());
	}

	void BlurJob::Render(const JobInput& jobInput)
	{
		effect->SetSettings(jobInput.renderingSettings.blurRadius);

		renderTarget->Begin();
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		// Todo: Should this be moved to the effect instead?
		commandBuffer->CmdBindPipeline(effect->GetPipeline());
		effect->BindDescriptorSets(commandBuffer);

		gRendererUtility().DrawFullscreenQuad(commandBuffer);

		renderTarget->End();
	}
}
