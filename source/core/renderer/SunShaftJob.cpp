#include "core/renderer/SunShaftJob.h"
#include "core/renderer/DeferredJob.h"
#include "core/renderer/SkydomeJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "Camera.h"

namespace Utopian
{
	SunShaftJob::SunShaftJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		mSunAzimuth = 0.0f;
	}

	SunShaftJob::~SunShaftJob()
	{
	}

	void SunShaftJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);
		SkydomeJob* skydomeJob = static_cast<SkydomeJob*>(jobs[JobGraph::SKYBOX_INDEX]);

		// Note: Todo: Probably don't need to be the native window size
		mRadialBlurRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
		mRadialBlurRenderTarget->AddReadWriteColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		mRadialBlurRenderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/sun_shafts/sun_shafts.frag";

		mRadialBlurEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRadialBlurRenderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		mRadialBlurEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mRadialBlurEffect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		// Enable additive blending
		gRendererUtility().SetAdditiveBlending(mRadialBlurEffect->GetPipeline());
		mRadialBlurEffect->CreatePipeline();

		mRadialBlurParameters.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mRadialBlurEffect->BindUniformBuffer("UBO_parameters", &mRadialBlurParameters);
		mRadialBlurEffect->BindCombinedImage("sunSampler", skydomeJob->sunImage, mRadialBlurRenderTarget->GetSampler());

		mSkydomeModel = Vk::gModelLoader().LoadModel("data/models/sphere.obj");
	}

	void SunShaftJob::Render(const JobInput& jobInput)
	{
		// Move sun
		mSunAzimuth += Timer::Instance().GetTime() / 10000000 * jobInput.renderingSettings.sunSpeed;

		// Calculate light direction
		float sunInclination = glm::radians(jobInput.renderingSettings.sunInclination);

		// Note: Todo: Why negative azimuth?
		mSunDir = glm::vec3(sin(sunInclination) * cos(-mSunAzimuth),
			cos(sunInclination),
			sin(sunInclination) * sin(-mSunAzimuth));

		// Calculate sun screen space position
		float skydomeRadius = mSkydomeModel->GetBoundingBox().GetWidth() / 2.0f;
		// Note: Todo: Why -X -Z here?
		// Note: Todo: Why - camera position?
		glm::vec3 sunWorldPos = glm::vec3(-1, 1, -1) * mSunDir * skydomeRadius * mSkydomeScale - jobInput.sceneInfo.eyePos;;
		glm::vec4 clipPos = jobInput.sceneInfo.projectionMatrix * jobInput.sceneInfo.viewMatrix * glm::vec4(sunWorldPos, 1.0f);
		glm::vec4 ndcPos = clipPos / clipPos.w;
		glm::vec2 texCoord = glm::vec2(ndcPos) / 2.0f + 0.5f;

		// Apply post process effect
		mRadialBlurParameters.data.radialOrigin = texCoord;
		mRadialBlurParameters.data.radialBlurScale = 1.0f;
		mRadialBlurParameters.data.radialBlurStrength = 1.0f;
		mRadialBlurParameters.UpdateMemory();

		mRadialBlurRenderTarget->Begin("Sun shaft pass", glm::vec4(0.7, 0.7, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = mRadialBlurRenderTarget->GetCommandBuffer();

		if (IsEnabled())
		{
			commandBuffer->CmdBindPipeline(mRadialBlurEffect->GetPipeline());
			commandBuffer->CmdBindDescriptorSets(mRadialBlurEffect);
			gRendererUtility().DrawFullscreenQuad(commandBuffer);
		}

		mRadialBlurRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
	}
}
