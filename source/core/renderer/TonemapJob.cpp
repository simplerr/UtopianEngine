#include "core/renderer/TonemapJob.h"
#include "core/renderer/DeferredJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/handles/Sampler.h"

namespace Utopian
{
	TonemapJob::TonemapJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		outputImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT);

		renderTarget = std::make_shared<Vk::RenderTarget>(device, width, height);
		renderTarget->AddWriteOnlyColorAttachment(outputImage);
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/post_process/tonemap.frag";
		effect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, renderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		effect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		effect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		effect->CreatePipeline();

		settingsBlock.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		effect->BindUniformBuffer("UBO_settings", &settingsBlock);
	}

	TonemapJob::~TonemapJob()
	{
	}

	void TonemapJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);

		sampler = std::make_shared<Vk::Sampler>(mDevice, false);
		sampler->createInfo.anisotropyEnable = VK_FALSE;
		sampler->Create();

		effect->BindCombinedImage("hdrSampler", deferredJob->renderTarget->GetColorImage().get(), sampler.get());
	}

	void TonemapJob::Render(const JobInput& jobInput)
	{
		settingsBlock.data.algorithm = 0;
		settingsBlock.data.exposure = jobInput.renderingSettings.exposure;
		settingsBlock.UpdateMemory();

		renderTarget->Begin("Tonemap pass", glm::vec4(0.5, 1.0, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		// Todo: Should this be moved to the effect instead?
		commandBuffer->CmdBindPipeline(effect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(effect);

		gRendererUtility().DrawFullscreenQuad(commandBuffer);

		renderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
	}
}
