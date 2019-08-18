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
		InitFirstPass(jobs, gbuffer);
		InitSecondPass(jobs, gbuffer);
	}

	void SSRJob::InitFirstPass(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);

		ssrImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16B16A16_SFLOAT);

		mCalculateRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
		mCalculateRenderTarget->AddWriteOnlyColorAttachment(ssrImage);
		mCalculateRenderTarget->SetClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		mCalculateRenderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/post_process/ssr_calculate.frag";

		mCalculateSSREffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mCalculateRenderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		mCalculateSSREffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mCalculateSSREffect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		mCalculateSSREffect->CreatePipeline();

		mCalculateSSREffect->BindCombinedImage("lightSampler", deferredJob->renderTarget->GetColorImage().get(), mCalculateRenderTarget->GetSampler());
		mCalculateSSREffect->BindCombinedImage("specularSampler", gbuffer.specularImage.get(), mCalculateRenderTarget->GetSampler());
		mCalculateSSREffect->BindCombinedImage("normalSampler", gbuffer.normalViewImage.get(), mCalculateRenderTarget->GetSampler());
		mCalculateSSREffect->BindCombinedImage("positionSampler", gbuffer.positionImage.get(), mCalculateRenderTarget->GetSampler());

		mUniformBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mCalculateSSREffect->BindUniformBuffer("UBO", &mUniformBlock);

		const uint32_t size = 440;
		gScreenQuadUi().AddQuad(1 * (size + 10) + 10, mHeight - (size + 100), size, size, ssrImage.get(), mCalculateRenderTarget->GetSampler());
	}

	void SSRJob::InitSecondPass(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);

		mApplyRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
		mApplyRenderTarget->AddReadWriteColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		mApplyRenderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/post_process/ssr_apply.frag";

		mApplySSREffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mApplyRenderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		mApplySSREffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mApplySSREffect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		// Enable additive blending
		gRendererUtility().SetAdditiveBlending(mApplySSREffect->GetPipeline());
		mApplySSREffect->CreatePipeline();

		mApplySSREffect->BindCombinedImage("reflectionSampler", ssrImage.get(), mApplyRenderTarget->GetSampler());
	}

	void SSRJob::Render(const JobInput& jobInput)
	{
		RenderFirstPass(jobInput);
		RenderSecondPass(jobInput);
	}

	void SSRJob::RenderFirstPass(const JobInput& jobInput)
	{
		mUniformBlock.data.view = jobInput.sceneInfo.viewMatrix;
		mUniformBlock.data.projection = jobInput.sceneInfo.projectionMatrix;
		mUniformBlock.UpdateMemory();

		mCalculateRenderTarget->Begin("SSR pass #1", glm::vec4(0.0, 1.0, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = mCalculateRenderTarget->GetCommandBuffer();

		if (IsEnabled())
		{
			commandBuffer->CmdBindPipeline(mCalculateSSREffect->GetPipeline());
			commandBuffer->CmdBindDescriptorSets(mCalculateSSREffect);
			gRendererUtility().DrawFullscreenQuad(commandBuffer);
		}

		mCalculateRenderTarget->End(GetWaitSemahore(), mFirstPassSemaphore);
	}

	void SSRJob::RenderSecondPass(const JobInput& jobInput)
	{
		mApplyRenderTarget->Begin("SSR pass #2", glm::vec4(0.0, 1.0, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = mApplyRenderTarget->GetCommandBuffer();

		if (IsEnabled())
		{
			commandBuffer->CmdBindPipeline(mApplySSREffect->GetPipeline());
			commandBuffer->CmdBindDescriptorSets(mApplySSREffect);
			gRendererUtility().DrawFullscreenQuad(commandBuffer);
		}

		mApplyRenderTarget->End(mFirstPassSemaphore, GetCompletedSemahore());
	}
}
