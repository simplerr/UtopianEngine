#include "core/renderer/BloomJob.h"
#include "core/renderer/DeferredJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/handles/Sampler.h"

namespace Utopian
{
	BloomJob::BloomJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		//mWidth = mWidth / 16.0;
		//mHeight = mHeight / 16.0;
		InitExtractPass();
		InitBlurPass();

		waitExtractPassSemaphore = std::make_shared<Vk::Semaphore>(mDevice);
	}

	BloomJob::~BloomJob()
	{
	}

	void BloomJob::InitExtractPass()
	{
		brightColorsImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth / OFFSCREEN_RATIO, mHeight / OFFSCREEN_RATIO, VK_FORMAT_R16G16B16A16_SFLOAT);

		extractRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth / OFFSCREEN_RATIO, mHeight / OFFSCREEN_RATIO);
		extractRenderTarget->AddWriteOnlyColorAttachment(brightColorsImage);
		extractRenderTarget->SetClearColor(1, 1, 1, 1);
		extractRenderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/post_process/bloom_extract.frag";
		extractEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, extractRenderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		extractEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		extractEffect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		extractEffect->CreatePipeline();

		extractSettings.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		extractEffect->BindUniformBuffer("UBO_settings", &extractSettings);

		//const uint32_t size = 240;
		//gScreenQuadUi().AddQuad(5 * (size + 10) + 10, mHeight - (size + 10), size, size, brightColorsImage.get(), extractRenderTarget->GetSampler());
	}
	
	void BloomJob::InitBlurPass()
	{
		outputImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth / OFFSCREEN_RATIO, mHeight / OFFSCREEN_RATIO, VK_FORMAT_R16G16B16A16_SFLOAT);

		blurRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth / OFFSCREEN_RATIO, mHeight / OFFSCREEN_RATIO);
		blurRenderTarget->AddWriteOnlyColorAttachment(outputImage);
		blurRenderTarget->SetClearColor(1, 1, 1, 1);
		blurRenderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/post_process/bloom_blur.frag";
		blurEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, blurRenderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		blurEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		blurEffect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		blurEffect->CreatePipeline();

		blurSettings.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		blurEffect->BindUniformBuffer("UBO_settings", &blurSettings);
		blurEffect->BindCombinedImage("hdrSampler", brightColorsImage.get(), blurRenderTarget->GetSampler());

		/*const uint32_t size = 240;
		gScreenQuadUi().AddQuad(5 * (size + 10) + 10, mHeight - (2 * size + 10), size, size, outputImage.get(), blurRenderTarget->GetSampler());*/
	}

	void BloomJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);

		extractEffect->BindCombinedImage("hdrSampler", deferredJob->renderTarget->GetColorImage().get(), extractRenderTarget->GetSampler());
	}

	void BloomJob::RenderExtractPass(const JobInput& jobInput)
	{
		extractSettings.data.threshold = jobInput.renderingSettings.bloomThreshold;
		extractSettings.UpdateMemory();

		extractRenderTarget->Begin("Bloom extract pass");
		Vk::CommandBuffer* commandBuffer = extractRenderTarget->GetCommandBuffer();

		// Todo: Should this be moved to the effect instead?
		commandBuffer->CmdBindPipeline(extractEffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(extractEffect);

		gRendererUtility().DrawFullscreenQuad(commandBuffer);

		extractRenderTarget->End(GetWaitSemahore(), waitExtractPassSemaphore);
	}
	
	void BloomJob::RenderBlurPass(const JobInput& jobInput)
	{
		blurSettings.data.size = 5;// = jobInput.renderingSettings.bloomThreshold;
		blurSettings.UpdateMemory();

		blurRenderTarget->Begin("Bloom blur pass");
		Vk::CommandBuffer* commandBuffer = blurRenderTarget->GetCommandBuffer();

		// Todo: Should this be moved to the effect instead?
		commandBuffer->CmdBindPipeline(blurEffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(blurEffect);

		gRendererUtility().DrawFullscreenQuad(commandBuffer);

		blurRenderTarget->End(waitExtractPassSemaphore, GetCompletedSemahore());
	}

	void BloomJob::Render(const JobInput& jobInput)
	{
		RenderExtractPass(jobInput);
		RenderBlurPass(jobInput);
	}
}
