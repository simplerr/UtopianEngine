#include "core/renderer/SSRJob.h"
#include "core/renderer/DeferredJob.h"
#include "core/renderer/GeometryThicknessJob.h"
#include "core/renderer/OpaqueCopyJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "utility/math/Helpers.h"
#include "ImGuiRenderer.h"
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
		ssrImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16B16A16_SFLOAT);
		rayOriginImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16B16A16_SFLOAT);
		rayEndImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16B16A16_SFLOAT);
		miscDebugImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16B16A16_SFLOAT);

		mTraceRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
		mTraceRenderTarget->AddWriteOnlyColorAttachment(ssrImage);
		mTraceRenderTarget->AddWriteOnlyColorAttachment(rayOriginImage);
		mTraceRenderTarget->AddWriteOnlyColorAttachment(rayEndImage);
		mTraceRenderTarget->AddWriteOnlyColorAttachment(miscDebugImage);
		mTraceRenderTarget->SetClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		mTraceRenderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/ssr/ssr.frag";

		mTraceSSREffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mTraceRenderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		mTraceSSREffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mTraceSSREffect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		mTraceSSREffect->CreatePipeline();

		DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);
		GeometryThicknessJob* geometryThicknessJob = static_cast<GeometryThicknessJob*>(jobs[JobGraph::GEOMETRY_THICKNESS_INDEX]);
		OpaqueCopyJob* opaqueCopyJob = static_cast<OpaqueCopyJob*>(jobs[JobGraph::OPAQUE_COPY_INDEX]);

		mTraceSSREffect->BindCombinedImage("_MainTex", deferredJob->renderTarget->GetColorImage().get(), mTraceRenderTarget->GetSampler());
		mTraceSSREffect->BindCombinedImage("_CameraDepthTexture", opaqueCopyJob->opaqueDepthImage.get(), mTraceRenderTarget->GetSampler());
		mTraceSSREffect->BindCombinedImage("_BackFaceDepthTex", geometryThicknessJob->geometryThicknessImage.get(), mTraceRenderTarget->GetSampler());
		mTraceSSREffect->BindCombinedImage("_CameraGBufferTexture1", gbuffer.specularImage.get(), mTraceRenderTarget->GetSampler());
		mTraceSSREffect->BindCombinedImage("_CameraGBufferTexture2", gbuffer.normalViewImage.get(), mTraceRenderTarget->GetSampler());
		mTraceSSREffect->BindCombinedImage("positionSampler", gbuffer.positionImage.get(), mTraceRenderTarget->GetSampler());
		mTraceSSREffect->BindCombinedImage("normalSampler", gbuffer.normalImage.get(), mTraceRenderTarget->GetSampler());

		mSSRSettingsBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mTraceSSREffect->BindUniformBuffer("UBO_ssrSettings", &mSSRSettingsBlock);

		mSkyParameterBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mTraceSSREffect->BindUniformBuffer("UBO_parameters", &mSkyParameterBlock);

		mSSRSettingsBlock.data._Iterations = 420;
		mSSRSettingsBlock.data._BinarySearchIterations = 1;
		mSSRSettingsBlock.data._PixelZSize = 0.0f;
		mSSRSettingsBlock.data._PixelStride = 1;
		mSSRSettingsBlock.data._PixelStrideZCuttoff = 100.0f;
		mSSRSettingsBlock.data._MaxRayDistance = 130000.0f;
		mSSRSettingsBlock.data._ScreenEdgeFadeStart = 0.90;
		mSSRSettingsBlock.data._EyeFadeStart = 0.0f;
		mSSRSettingsBlock.data._EyeFadeEnd = 1.0f;
		mSSRSettingsBlock.data._GeometryThickness = 20.0f;

		// const uint32_t size = 640;
		// gScreenQuadUi().AddQuad(100, 100, size, size, ssrImage.get(), mTraceRenderTarget->GetSampler());
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
		shaderCreateInfo.fragmentShaderPath = "data/shaders/ssr/ssr_blur.frag";

		mBlurSSREffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mBlurRenderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		mBlurSSREffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mBlurSSREffect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		mBlurSSREffect->CreatePipeline();
		mBlurSSREffect->BindCombinedImage("samplerSSAO", ssrImage.get(), mTraceRenderTarget->GetSampler());

		// const uint32_t size = 640;
		// gScreenQuadUi().AddQuad(100, 100, size, size, ssrBlurImage.get(), mBlurRenderTarget->GetSampler());
	}

	void SSRJob::Render(const JobInput& jobInput)
	{
		RenderTracePass(jobInput);
		RenderBlurPass(jobInput);
	}

	void SSRJob::RenderTracePass(const JobInput& jobInput)
	{
		mSSRSettingsBlock.data._CameraProjectionMatrix = jobInput.sceneInfo.projectionMatrix;
		mSSRSettingsBlock.data._CameraInverseProjectionMatrix = glm::inverse(glm::mat3(jobInput.sceneInfo.projectionMatrix));
		mSSRSettingsBlock.data._NormalMatrix = glm::transpose(glm::inverse(glm::mat3(jobInput.sceneInfo.viewMatrix)));
		mSSRSettingsBlock.data._RenderBufferSize = glm::vec2(mWidth, mHeight);
		mSSRSettingsBlock.data._OneDividedByRenderBufferSize = glm::vec2(1.0f / mWidth, 1.0f / mHeight); 
		mSSRSettingsBlock.data._ViewMatrix = jobInput.sceneInfo.viewMatrix;
		mSSRSettingsBlock.data._SSREnabled = jobInput.renderingSettings.ssrEnabled;
		mSSRSettingsBlock.UpdateMemory();

		// Note: Todo: Move to common location
		mSkyParameterBlock.data.inclination = 0.7853981850f;
		mSkyParameterBlock.data.azimuth = 0.0f;
		mSkyParameterBlock.data.time = Timer::Instance().GetTime();
		mSkyParameterBlock.data.sunSpeed = jobInput.renderingSettings.sunSpeed;
		mSkyParameterBlock.data.eyePos = jobInput.sceneInfo.eyePos;
		mSkyParameterBlock.data.onlySun = false;
		mSkyParameterBlock.UpdateMemory();

		mTraceRenderTarget->Begin("SSR trace pass", glm::vec4(0.0, 1.0, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = mTraceRenderTarget->GetCommandBuffer();

		commandBuffer->CmdBindPipeline(mTraceSSREffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(mTraceSSREffect);
		gRendererUtility().DrawFullscreenQuad(commandBuffer);

		mTraceRenderTarget->End(GetWaitSemahore(), mTracePassSemaphore);
	}

	void SSRJob::RenderBlurPass(const JobInput& jobInput)
	{
		mBlurRenderTarget->Begin("SSR blur pass", glm::vec4(0.0, 1.0, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = mBlurRenderTarget->GetCommandBuffer();

		commandBuffer->CmdBindPipeline(mBlurSSREffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(mBlurSSREffect);
		gRendererUtility().DrawFullscreenQuad(commandBuffer);

		mBlurRenderTarget->End(mTracePassSemaphore, GetCompletedSemahore());
	}

	void SSRJob::Update()
	{
		ImGuiRenderer::BeginWindow("SSR settings", glm::vec2(500.0f, 10.0f), 400.0f);
		ImGui::SliderFloat("_Iterations: ", &mSSRSettingsBlock.data._Iterations, 1, 1000);
		ImGui::SliderFloat("_BinarySearchIterations", &mSSRSettingsBlock.data._BinarySearchIterations, 1, 32);
		ImGui::SliderFloat("_PixelZSize: ", &mSSRSettingsBlock.data._PixelZSize, 0.0f, 2.0f);
		ImGui::SliderFloat("_PixelStride: ", &mSSRSettingsBlock.data._PixelStride, 1.0f, 64.0f);
		ImGui::SliderFloat("_PixelStrideZCuttoff: ", &mSSRSettingsBlock.data._PixelStrideZCuttoff, 0.0f, 200.0f);
		ImGui::SliderFloat("_MaxRayDistance: ", &mSSRSettingsBlock.data._MaxRayDistance, 0.0f, 400000.0f);
		ImGui::SliderFloat("_ScreenEdgeFadeStart: ", &mSSRSettingsBlock.data._ScreenEdgeFadeStart, 0.0f, 1.0f);
		ImGui::SliderFloat("_EyeFadeStart: ", &mSSRSettingsBlock.data._EyeFadeStart, 0.0f, 1.0f);
		ImGui::SliderFloat("_EyeFadeEnd: ", &mSSRSettingsBlock.data._EyeFadeEnd, 0.0f, 1.0f);
		ImGui::SliderFloat("_GeometryThickness: ", &mSSRSettingsBlock.data._GeometryThickness, 0.1f, 100.0f);
		ImGuiRenderer::EndWindow();
	}
}