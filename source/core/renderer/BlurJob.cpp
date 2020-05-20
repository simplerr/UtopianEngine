#include "core/renderer/BlurJob.h"
#include "core/renderer/SSAOJob.h"
#include "core/renderer/CommonJobIncludes.h"

namespace Utopian
{
	BlurJob::BlurJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		blurImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT);

		mRenderTarget = std::make_shared<Vk::RenderTarget>(device, width, height);
		mRenderTarget->AddWriteOnlyColorAttachment(blurImage);
		mRenderTarget->SetClearColor(1, 1, 1, 1);
		mRenderTarget->Create();

		mEffect = Vk::gEffectManager().AddEffect<Vk::BlurEffect>(device, mRenderTarget->GetRenderPass());

		/*const uint32_t size = 240;
		gScreenQuadUi().AddQuad(10, height - (size + 10), size, size, blurImage.get(), renderTarget->GetSampler());*/
	}

	BlurJob::~BlurJob()
	{
	}

	void BlurJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		SSAOJob* ssaoJob = static_cast<SSAOJob*>(jobs[JobGraph::SSAO_INDEX]);

		mEffect->BindCombinedImage("samplerSSAO", *ssaoJob->ssaoImage, *ssaoJob->renderTarget->GetSampler());
	}

	void BlurJob::Render(const JobInput& jobInput)
	{
		mEffect->SetSettings(jobInput.renderingSettings.blurRadius);

		mRenderTarget->Begin("SSAO blur pass", glm::vec4(0.5, 1.0, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = mRenderTarget->GetCommandBuffer();

		// Todo: Should this be moved to the effect instead?
		commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(mEffect);

		gRendererUtility().DrawFullscreenQuad(commandBuffer);

		mRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
	}
}
