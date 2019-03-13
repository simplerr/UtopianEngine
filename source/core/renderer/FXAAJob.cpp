#include "core/renderer/FXAAJob.h"
#include "core/renderer/DeferredJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/handles/Sampler.h"

namespace Utopian
{
	FXAAJob::FXAAJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		fxaaImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT);

		renderTarget = std::make_shared<Vk::RenderTarget>(device, width, height);
		renderTarget->AddWriteOnlyColorAttachment(fxaaImage);
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/fxaa/fxaa.frag";
		effect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, renderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		effect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		effect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		effect->CreatePipeline();

		gScreenQuadUi().AddQuad(0u, 0u, width, height, fxaaImage.get(), renderTarget->GetSampler(), 1u);
	}

	FXAAJob::~FXAAJob()
	{
	}

	void FXAAJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);

		sampler = std::make_shared<Vk::Sampler>(mDevice, false);
		sampler->createInfo.anisotropyEnable = VK_FALSE;
		sampler->Create();

		effect->BindCombinedImage("textureSampler", deferredJob->renderTarget->GetColorImage().get(), sampler.get());
	}

	void FXAAJob::Render(const JobInput& jobInput)
	{
		renderTarget->Begin("FXAA pass", glm::vec4(0.5, 1.0, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		// Todo: Should this be moved to the effect instead?
		commandBuffer->CmdBindPipeline(effect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(effect);

		gRendererUtility().DrawFullscreenQuad(commandBuffer);

		renderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
	}
}
