#include "core/renderer/SSRJob.h"
#include "core/renderer/DeferredJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "utility/math/Helpers.h"
#include <random>

namespace Utopian
{
	SSRJob::SSRJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
	}

	SSRJob::~SSRJob()
	{
	}

	void SSRJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		InitTracePass(jobs, gbuffer);
		InitBlurPass(jobs, gbuffer);
	}

	void SSRJob::InitTracePass(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);

		ssrImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16B16A16_SFLOAT);

		mTraceRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
		mTraceRenderTarget->AddWriteOnlyColorAttachment(ssrImage);
		mTraceRenderTarget->SetClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		mTraceRenderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/post_process/ssr_trace_vkdf.frag";

		mTraceSSREffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mTraceRenderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		mTraceSSREffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mTraceSSREffect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		mTraceSSREffect->CreatePipeline();

		mTraceSSREffect->BindCombinedImage("lightSampler", deferredJob->renderTarget->GetColorImage().get(), mTraceRenderTarget->GetSampler());
		mTraceSSREffect->BindCombinedImage("specularSampler", gbuffer.albedoImage.get(), mTraceRenderTarget->GetSampler());
		mTraceSSREffect->BindCombinedImage("normalViewSampler", gbuffer.normalViewImage.get(), mTraceRenderTarget->GetSampler());
		mTraceSSREffect->BindCombinedImage("normalWorldSampler", gbuffer.normalImage.get(), mTraceRenderTarget->GetSampler());
		mTraceSSREffect->BindCombinedImage("positionSampler", gbuffer.positionImage.get(), mTraceRenderTarget->GetSampler());
		mTraceSSREffect->BindCombinedImage("depthSampler", gbuffer.depthImage.get(), mTraceRenderTarget->GetSampler());

		mUniformBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mTraceSSREffect->BindUniformBuffer("UBO", &mUniformBlock);

		const uint32_t size = 640;
		gScreenQuadUi().AddQuad(100, 100, size, size, ssrImage.get(), mTraceRenderTarget->GetSampler());
	}

	void SSRJob::InitBlurPass(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);

		ssrBlurImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16B16A16_SFLOAT);

		mBlurRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
		mBlurRenderTarget->AddWriteOnlyColorAttachment(ssrBlurImage);
		mBlurRenderTarget->SetClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		mBlurRenderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/post_process/ssr_blur.frag";

		mBlurSSREffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mBlurRenderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		mBlurSSREffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mBlurSSREffect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		mBlurSSREffect->CreatePipeline();
		mBlurSSREffect->BindCombinedImage("samplerSSAO", ssrImage.get(), mTraceRenderTarget->GetSampler());

		// const uint32_t size = 640;
		// gScreenQuadUi().AddQuad(100 + 700, 100, size, size, ssrBlurImage.get(), mBlurRenderTarget->GetSampler());
	}

	void SSRJob::Render(const JobInput& jobInput)
	{
		RenderTracePass(jobInput);
		RenderBlurPass(jobInput);
	}

	void SSRJob::RenderTracePass(const JobInput& jobInput)
	{
		mUniformBlock.data.view = jobInput.sceneInfo.viewMatrix;
		mUniformBlock.data.projection = jobInput.sceneInfo.projectionMatrix;
		mUniformBlock.UpdateMemory();

		mTraceRenderTarget->Begin("SSR trace pass", glm::vec4(0.0, 1.0, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = mTraceRenderTarget->GetCommandBuffer();

		if (IsEnabled())
		{
			commandBuffer->CmdBindPipeline(mTraceSSREffect->GetPipeline());
			commandBuffer->CmdBindDescriptorSets(mTraceSSREffect);
			gRendererUtility().DrawFullscreenQuad(commandBuffer);
		}

		mTraceRenderTarget->End(GetWaitSemahore(), mTracePassSemaphore);
	}

	void SSRJob::RenderBlurPass(const JobInput& jobInput)
	{
		mBlurRenderTarget->Begin("SSR blur pass", glm::vec4(0.0, 1.0, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = mBlurRenderTarget->GetCommandBuffer();

		if (IsEnabled())
		{
			commandBuffer->CmdBindPipeline(mBlurSSREffect->GetPipeline());
			commandBuffer->CmdBindDescriptorSets(mBlurSSREffect);
			gRendererUtility().DrawFullscreenQuad(commandBuffer);
		}

		mBlurRenderTarget->End(mTracePassSemaphore, GetCompletedSemahore());
	}
}