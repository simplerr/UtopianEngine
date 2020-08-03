#include "core/renderer/jobs/TonemapJob.h"
#include "core/renderer/jobs/DeferredJob.h"
#include "core/renderer/jobs/BloomJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/handles/Sampler.h"

namespace Utopian
{
	TonemapJob::TonemapJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		outputImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT, "Tonemap image");

		mRenderTarget = std::make_shared<Vk::RenderTarget>(device, width, height);
		mRenderTarget->AddWriteOnlyColorAttachment(outputImage);
		mRenderTarget->SetClearColor(1, 1, 1, 1);
		mRenderTarget->Create();

		Vk::EffectCreateInfo effectDesc;
		effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/post_process/tonemap.frag";

		mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDesc);

		mSettingsBlock.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mEffect->BindUniformBuffer("UBO_settings", mSettingsBlock);
	}

	TonemapJob::~TonemapJob()
	{
	}

	void TonemapJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);
		BloomJob* bloomJob = static_cast<BloomJob*>(jobs[JobGraph::BLOOM_INDEX]);

		mSampler = std::make_shared<Vk::Sampler>(mDevice, false);
		mSampler->createInfo.anisotropyEnable = VK_FALSE;
		mSampler->Create();

		mEffect->BindCombinedImage("hdrSampler", *deferredJob->renderTarget->GetColorImage(), *mSampler);
		mEffect->BindCombinedImage("bloomSampler", *bloomJob->outputImage, *mSampler);
	}

	void TonemapJob::Render(const JobInput& jobInput)
	{
		mSettingsBlock.data.tonemapping = jobInput.renderingSettings.tonemapping;
		mSettingsBlock.data.exposure = jobInput.renderingSettings.exposure;
		mSettingsBlock.UpdateMemory();

		mRenderTarget->Begin("Tonemap pass", glm::vec4(0.5, 1.0, 1.0, 1.0));
		Vk::CommandBuffer* commandBuffer = mRenderTarget->GetCommandBuffer();

		// Todo: Should this be moved to the effect instead?
		commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(mEffect);

		gRendererUtility().DrawFullscreenQuad(commandBuffer);

		mRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
	}
}
