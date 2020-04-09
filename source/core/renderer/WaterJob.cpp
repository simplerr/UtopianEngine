#include "core/renderer/WaterJob.h"
#include "core/renderer/DeferredJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "core/renderer/Renderer.h"
#include "vulkan/ShaderFactory.h"
#include "ImGuiRenderer.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/TextureLoader.h"
#include "ScreenQuadRenderer.h"
#include "vulkan/Vertex.h"
#include "vulkan/handles/QueryPool.h"
#include "Camera.h"
#include "Input.h"
#include <random>

namespace Utopian
{
	WaterJob::WaterJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
        mWaterMesh = GeneratePatches(128.0, 64);
	}

	WaterJob::~WaterJob()
	{
        delete mWaterMesh;
	}

	void WaterJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
        InitCopyPass(jobs, gbuffer);

		DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);

		testImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16B16A16_SFLOAT);

		renderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
		renderTarget->AddReadWriteColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->AddReadWriteColorAttachment(gbuffer.positionImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->AddReadWriteColorAttachment(gbuffer.normalImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->AddReadWriteColorAttachment(gbuffer.albedoImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->AddReadWriteColorAttachment(gbuffer.normalViewImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->AddWriteOnlyColorAttachment(testImage);
		renderTarget->AddReadWriteDepthAttachment(gbuffer.depthImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/tessellation/water.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/tessellation/water.frag";
		shaderCreateInfo.tescShaderPath = "data/shaders/tessellation/water.tesc";
		shaderCreateInfo.teseShaderPath = "data/shaders/tessellation/water.tese";
		shaderCreateInfo.geometryShaderPath = "data/shaders/tessellation/water.geom";
		mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, renderTarget->GetRenderPass(), shaderCreateInfo);

		//mEffect->GetPipeline()->rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		mEffect->GetPipeline()->inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		mEffect->GetPipeline()->AddTessellationState(4);

		mEffect->CreatePipeline();

		mViewProjectionBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mEffect->BindUniformBuffer("UBO_viewProjection", &mViewProjectionBlock);

		mSettingsBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mEffect->BindUniformBuffer("UBO_settings", &mSettingsBlock);

		mSkyParameterBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mEffect->BindUniformBuffer("UBO_parameters", &mSkyParameterBlock);

		mQueryPool = std::make_shared<Vk::QueryPool>(mDevice);

		// const uint32_t size = 640;
		// gScreenQuadUi().AddQuad(100 + 640, 100, size, size, testImage.get(), renderTarget->GetSampler());
	}

    void WaterJob::InitCopyPass(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
    {
		DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);

		originalDiffuseImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16B16A16_SFLOAT);
		originalDepthImage = std::make_shared<Vk::ImageDepth>(mDevice, mWidth, mHeight, VK_FORMAT_D32_SFLOAT_S8_UINT);

		mCopyRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
		mCopyRenderTarget->AddWriteOnlyColorAttachment(originalDiffuseImage);
		mCopyRenderTarget->AddWriteOnlyDepthAttachment(originalDepthImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		mCopyRenderTarget->SetClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		mCopyRenderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/post_process/gbuffer_copy.frag";

		mCopyEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mCopyRenderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		mCopyEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mCopyEffect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		mCopyEffect->CreatePipeline();

		mCopyEffect->BindCombinedImage("lightSampler", deferredJob->renderTarget->GetColorImage().get(), mCopyRenderTarget->GetSampler());
		mCopyEffect->BindCombinedImage("depthSampler", gbuffer.depthImage.get(), mCopyRenderTarget->GetSampler());
    }

	void WaterJob::RenderCopyPass(const JobInput& jobInput)
    {
		mCopyRenderTarget->Begin("Water copy pass", glm::vec4(0.0, 1.0, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = mCopyRenderTarget->GetCommandBuffer();

		if (IsEnabled())
		{
			commandBuffer->CmdBindPipeline(mCopyEffect->GetPipeline());
			commandBuffer->CmdBindDescriptorSets(mCopyEffect);
			gRendererUtility().DrawFullscreenQuad(commandBuffer);
		}

		mCopyRenderTarget->End(GetWaitSemahore(), mCopyPassSemaphore);
    }

	void WaterJob::Render(const JobInput& jobInput)
	{
        RenderCopyPass(jobInput);

		mViewProjectionBlock.data.view = jobInput.sceneInfo.viewMatrix;
		mViewProjectionBlock.data.projection = jobInput.sceneInfo.projectionMatrix;
		mViewProjectionBlock.data.time = gTimer().GetTime();
		mViewProjectionBlock.data.eyePos = gRenderer().GetMainCamera()->GetPosition();

		const Frustum& frustum = gRenderer().GetMainCamera()->GetFrustum();
		memcpy(mViewProjectionBlock.data.frustumPlanes, frustum.planes.data(), sizeof(glm::vec4) * 6);

		mViewProjectionBlock.UpdateMemory();

		mSettingsBlock.data.viewportSize = glm::vec2(mWidth, mHeight);
		mSettingsBlock.data.tessellationFactor = jobInput.renderingSettings.tessellationFactor;
		mSettingsBlock.data.edgeSize = 200.0f;
		mSettingsBlock.data.amplitude = jobInput.sceneInfo.terrain->GetAmplitudeScaling();
		mSettingsBlock.data.textureScaling = jobInput.renderingSettings.terrainTextureScaling;
		mSettingsBlock.data.bumpmapAmplitude = jobInput.renderingSettings.terrainBumpmapAmplitude;
		mSettingsBlock.data.wireframe = jobInput.renderingSettings.terrainWireframe;
		mSettingsBlock.UpdateMemory();

        // Note: Todo: Move to common location
		mSkyParameterBlock.data.inclination = 0.7853981850f;
		mSkyParameterBlock.data.azimuth = 0.0f;
		mSkyParameterBlock.data.time = Timer::Instance().GetTime();
		mSkyParameterBlock.data.sunSpeed = jobInput.renderingSettings.sunSpeed;
		mSkyParameterBlock.data.eyePos = jobInput.sceneInfo.eyePos;
		mSkyParameterBlock.data.onlySun = false;
		mSkyParameterBlock.UpdateMemory();

		renderTarget->BeginCommandBuffer("Water Tessellation pass");
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		mQueryPool->Reset(commandBuffer);

		renderTarget->BeginRenderPass();

		if (IsEnabled())
		{
			mQueryPool->Begin(commandBuffer);

			glm::mat4 world = glm::mat4();
			Vk::PushConstantBlock pushConsts(world);
			commandBuffer->CmdPushConstants(mEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConsts), &pushConsts);

			commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
			commandBuffer->CmdBindDescriptorSets(mEffect);

			commandBuffer->CmdBindVertexBuffer(0, 1, mWaterMesh->GetVertxBuffer());
			commandBuffer->CmdBindIndexBuffer(mWaterMesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
			commandBuffer->CmdDrawIndexed(mWaterMesh->GetNumIndices(), 1, 0, 0, 0);

			mQueryPool->End(commandBuffer);
		}

		renderTarget->End(mCopyPassSemaphore, GetCompletedSemahore());

		mQueryPool->RetreiveResults();
	}

	void WaterJob::Update()
	{
		// Display Actor creation list
		ImGuiRenderer::BeginWindow("Water Tessellation statistics", glm::vec2(300.0f, 10.0f), 400.0f);

		ImGuiRenderer::TextV("VS invocations: %u", mQueryPool->GetStatistics(Vk::QueryPool::StatisticsIndex::INPUT_ASSEMBLY_VERTICES_INDEX));
		ImGuiRenderer::TextV("TC invocations: %u", mQueryPool->GetStatistics(Vk::QueryPool::StatisticsIndex::TESSELLATION_CONTROL_SHADER_PATCHES_INDEX));
		ImGuiRenderer::TextV("TE invocations: %u", mQueryPool->GetStatistics(Vk::QueryPool::StatisticsIndex::TESSELLATION_EVALUATION_SHADER_INVOCATIONS_INDEX));
		ImGuiRenderer::TextV("FS invocations: %u", mQueryPool->GetStatistics(Vk::QueryPool::StatisticsIndex::FRAGMENT_SHADER_INVOCATIONS_INDEX));

		ImGuiRenderer::EndWindow();
	}

	Vk::Mesh* WaterJob::GeneratePatches(float cellSize, int numCells)
	{
		Vk::Mesh* mesh = new Vk::Mesh(mDevice);

		// Vertices
		for (auto x = 0; x < numCells; x++)
		{
			for (auto z = 0; z < numCells; z++)
			{
				Vk::Vertex vertex;
				const float originOffset = (float)numCells * cellSize / 2.0f;
				vertex.Pos = glm::vec3(x * cellSize + cellSize / 2.0f - originOffset, 0.0f, z * cellSize + cellSize / 2.0f - originOffset);
				vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
				vertex.Tex = glm::vec2((float)x / (numCells - 1), (float)z / (numCells - 1));
				mesh->AddVertex(vertex);
			}
		}

		// Indices
		const uint32_t w = (numCells - 1);
		for (auto x = 0; x < w; x++)
		{
			for (auto z = 0; z < w; z++)
			{
				uint32_t v1 = (x + z * numCells);
				uint32_t v2 = v1 + numCells;
				uint32_t v3 = v2 + 1;
				uint32_t v4 = v1 + 1;
				mesh->AddQuad(v1, v2, v3, v4);
			}
		}

		mesh->BuildBuffers(mDevice);

        return mesh;
	}
}
