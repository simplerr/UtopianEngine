#include "core/renderer/SunShaftJob.h"
#include "core/renderer/DeferredJob.h"
#include "core/renderer/SkydomeJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "Camera.h"

namespace Utopian
{
	SunShaftJob::SunShaftJob(Vk::Renderer* renderer, uint32_t width, uint32_t height)
		: BaseJob(renderer, width, height)
	{
		sunAzimuth = 0.0f;
	}

	SunShaftJob::~SunShaftJob()
	{
	}

	void SunShaftJob::Init(const std::vector<BaseJob*>& jobs)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[RenderingManager::DEFERRED_INDEX]);
		SkydomeJob* skydomeJob = static_cast<SkydomeJob*>(jobs[RenderingManager::SKYBOX_INDEX]);

		// Note: Todo: Probably don't need to be the native window size
		radialBlurRenderTarget = std::make_shared<Vk::RenderTarget>(mRenderer->GetDevice(), mWidth, mHeight);
		radialBlurRenderTarget->AddColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ATTACHMENT_LOAD_OP_LOAD);
		radialBlurRenderTarget->Create();

		radialBlurEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mRenderer->GetDevice(),
			radialBlurRenderTarget->GetRenderPass(),
			"data/shaders/common/fullscreen.vert",
			"data/shaders/sun_shafts/sun_shafts.frag");

		// Vertices generated in fullscreen.vert are in clockwise order
		radialBlurEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		radialBlurEffect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		// Enable additive blending
		radialBlurEffect->GetPipeline()->blendAttachmentState[0].blendEnable = VK_TRUE;
		radialBlurEffect->GetPipeline()->blendAttachmentState[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		radialBlurEffect->GetPipeline()->blendAttachmentState[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		radialBlurEffect->GetPipeline()->blendAttachmentState[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		radialBlurEffect->GetPipeline()->blendAttachmentState[0].colorBlendOp = VK_BLEND_OP_ADD;
		radialBlurEffect->GetPipeline()->blendAttachmentState[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		radialBlurEffect->GetPipeline()->blendAttachmentState[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
		radialBlurEffect->GetPipeline()->blendAttachmentState[0].alphaBlendOp = VK_BLEND_OP_ADD;
		radialBlurEffect->CreatePipeline();

		radialBlurParameters.Create(mRenderer->GetDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		radialBlurEffect->BindUniformBuffer("UBO_parameters", &radialBlurParameters);
		radialBlurEffect->BindCombinedImage("sunSampler", skydomeJob->sunImage.get(), radialBlurRenderTarget->GetSampler());

		mSkydomeModel = Vk::gModelLoader().LoadModel("data/models/sphere.obj");
	}

	void SunShaftJob::Render(Vk::Renderer* renderer, const JobInput& jobInput)
	{
		// Move sun
		sunAzimuth += Timer::Instance().GetTime() / 10000000 * jobInput.renderingSettings.sunSpeed;

		// Calculate light direction
		float sunInclination = glm::radians(jobInput.renderingSettings.sunInclination);

		// Note: Todo: Why negative azimuth?
		sunDir = glm::vec3(sin(sunInclination) * cos(-sunAzimuth),
			cos(sunInclination),
			sin(sunInclination) * sin(-sunAzimuth));

		// Calculate sun screen space position
		float skydomeRadius = mSkydomeModel->GetBoundingBox().GetWidth() / 2.0f;
		// Note: Todo: Why -X -Z here?
		// Note: Todo: Why - camera position?
		glm::vec3 sunWorldPos = glm::vec3(-1, 1, -1) * sunDir * skydomeRadius * skydomeScale - jobInput.sceneInfo.eyePos;;
		glm::vec4 clipPos = jobInput.sceneInfo.projectionMatrix * jobInput.sceneInfo.viewMatrix * glm::vec4(sunWorldPos, 1.0f);
		glm::vec4 ndcPos = clipPos / clipPos.w;
		glm::vec2 texCoord = glm::vec2(ndcPos) / 2.0f + 0.5f;

		// Apply post process effect
		radialBlurParameters.data.radialOrigin = texCoord;
		radialBlurParameters.data.radialBlurScale = 1.0f;
		radialBlurParameters.data.radialBlurStrength = 1.0f;
		radialBlurParameters.UpdateMemory();

		radialBlurRenderTarget->Begin();
		Vk::CommandBuffer* commandBuffer = radialBlurRenderTarget->GetCommandBuffer();

		commandBuffer->CmdBindPipeline(radialBlurEffect->GetPipeline());
		radialBlurEffect->BindDescriptorSets(commandBuffer);

		gRendererUtility().DrawFullscreenQuad(commandBuffer);

		radialBlurRenderTarget->End();
	}
}
