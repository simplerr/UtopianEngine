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
		//InitTracePass(jobs, gbuffer);
		InitTracePassKode80(jobs, gbuffer);
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
		mTraceSSREffect->BindCombinedImage("normalViewSampler", gbuffer.normalViewImage.get(), mTraceRenderTarget->GetSampler());
		mTraceSSREffect->BindCombinedImage("normalWorldSampler", gbuffer.normalImage.get(), mTraceRenderTarget->GetSampler());
		mTraceSSREffect->BindCombinedImage("positionSampler", gbuffer.positionImage.get(), mTraceRenderTarget->GetSampler());
		mTraceSSREffect->BindCombinedImage("depthSampler", gbuffer.depthImage.get(), mTraceRenderTarget->GetSampler());
		mTraceSSREffect->BindCombinedImage("specularSampler", gbuffer.specularImage.get(), mTraceRenderTarget->GetSampler());

		mUniformBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mTraceSSREffect->BindUniformBuffer("UBO", &mUniformBlock);

		mSkyParameterBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mTraceSSREffect->BindUniformBuffer("UBO_parameters", &mSkyParameterBlock);

		mReflectionSettingsBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mTraceSSREffect->BindUniformBuffer("UBO_settings", &mReflectionSettingsBlock);

		// const uint32_t size = 640;
		// gScreenQuadUi().AddQuad(100, 100, size, size, ssrImage.get(), mTraceRenderTarget->GetSampler());
	}

	void SSRJob::InitTracePassKode80(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
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
		shaderCreateInfo.fragmentShaderPath = "data/shaders/post_process/ssr_kode80.frag";

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

		mKode80SettingsBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mTraceSSREffect->BindUniformBuffer("UBO_ssrSettings", &mKode80SettingsBlock);

		mSkyParameterBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mTraceSSREffect->BindUniformBuffer("UBO_parameters", &mSkyParameterBlock);

		const uint32_t size = 640;
		gScreenQuadUi().AddQuad(100, 100, size, size, ssrImage.get(), mTraceRenderTarget->GetSampler());

		mKode80SettingsBlock.data._Iterations = 300;
		mKode80SettingsBlock.data._BinarySearchIterations = 1;
		mKode80SettingsBlock.data._PixelZSize = 0.0f;            
		mKode80SettingsBlock.data._PixelStride = 1;           
		mKode80SettingsBlock.data._PixelStrideZCuttoff = 100.0f;   
		mKode80SettingsBlock.data._MaxRayDistance = 10000.0f;        
		mKode80SettingsBlock.data._ScreenEdgeFadeStart = 0.75f;   
		mKode80SettingsBlock.data._EyeFadeStart = 0.0f;          
		mKode80SettingsBlock.data._EyeFadeEnd = 1.0f;            
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
		//RenderTracePass(jobInput);
		RenderTracePassKode80(jobInput);
		RenderBlurPass(jobInput);
	}

	void SSRJob::RenderTracePass(const JobInput& jobInput)
	{
		mUniformBlock.data.view = jobInput.sceneInfo.viewMatrix;
		mUniformBlock.data.projection = jobInput.sceneInfo.projectionMatrix;
		mUniformBlock.UpdateMemory();

        // Note: Todo: Move to common location
		mSkyParameterBlock.data.inclination = 0.7853981850f;
		mSkyParameterBlock.data.azimuth = 0.0f;
		mSkyParameterBlock.data.time = Timer::Instance().GetTime();
		mSkyParameterBlock.data.sunSpeed = jobInput.renderingSettings.sunSpeed;
		mSkyParameterBlock.data.eyePos = jobInput.sceneInfo.eyePos;
		mSkyParameterBlock.data.onlySun = false;
		mSkyParameterBlock.UpdateMemory();
		
		mReflectionSettingsBlock.data.ssrEnabled = jobInput.renderingSettings.ssrEnabled;
		mReflectionSettingsBlock.data.skyboxReflections = jobInput.renderingSettings.skyboxReflections;
		mReflectionSettingsBlock.UpdateMemory();

		mTraceRenderTarget->Begin("SSR trace pass", glm::vec4(0.0, 1.0, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = mTraceRenderTarget->GetCommandBuffer();

		commandBuffer->CmdBindPipeline(mTraceSSREffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(mTraceSSREffect);
		gRendererUtility().DrawFullscreenQuad(commandBuffer);

		mTraceRenderTarget->End(GetWaitSemahore(), mTracePassSemaphore);
	}

	void SSRJob::RenderTracePassKode80(const JobInput& jobInput)
	{
		mKode80SettingsBlock.data._CameraProjectionMatrix = jobInput.sceneInfo.projectionMatrix;
		mKode80SettingsBlock.data._CameraInverseProjectionMatrix = glm::inverse(glm::mat3(jobInput.sceneInfo.projectionMatrix));
		mKode80SettingsBlock.data._NormalMatrix = glm::transpose(glm::inverse(glm::mat3(jobInput.sceneInfo.viewMatrix)));
		mKode80SettingsBlock.data._RenderBufferSize = glm::vec2(mWidth, mHeight);
		mKode80SettingsBlock.data._OneDividedByRenderBufferSize = glm::vec2(1.0f / mWidth, 1.0f / mHeight); 
		mKode80SettingsBlock.data.viewMatrix = jobInput.sceneInfo.viewMatrix;
		mKode80SettingsBlock.UpdateMemory();

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
		ImGui::SliderFloat("_Iterations: ", &mKode80SettingsBlock.data._Iterations, 1, 1000);
		ImGui::SliderFloat("_BinarySearchIterations", &mKode80SettingsBlock.data._BinarySearchIterations, 1, 32);
		ImGui::SliderFloat("_PixelZSize: ", &mKode80SettingsBlock.data._PixelZSize, 0.0f, 2.0f);
		ImGui::SliderFloat("_PixelStride: ", &mKode80SettingsBlock.data._PixelStride, 1.0f, 64.0f);
		ImGui::SliderFloat("_PixelStrideZCuttoff: ", &mKode80SettingsBlock.data._PixelStrideZCuttoff, 0.0f, 200.0f);
		ImGui::SliderFloat("_MaxRayDistance: ", &mKode80SettingsBlock.data._MaxRayDistance, 0.0f, 400000.0f);
		ImGui::SliderFloat("_ScreenEdgeFadeStart: ", &mKode80SettingsBlock.data._ScreenEdgeFadeStart, 0.0f, 1.0f);
		ImGui::SliderFloat("_EyeFadeStart: ", &mKode80SettingsBlock.data._EyeFadeStart, 0.0f, 1.0f);
		ImGui::SliderFloat("_EyeFadeEnd: ", &mKode80SettingsBlock.data._EyeFadeEnd, 0.0f, 1.0f);
		ImGuiRenderer::EndWindow();
	}
}