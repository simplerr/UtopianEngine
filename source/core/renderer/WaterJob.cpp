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
#include <glm/gtc/matrix_transform.hpp>

namespace Utopian
{
	WaterJob::WaterJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
        mWaterMesh = GeneratePatches(128.0f, 512);
	}

	WaterJob::~WaterJob()
	{
        delete mWaterMesh;
	}

	void WaterJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);

		distortionImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16_SFLOAT);

		renderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
		renderTarget->AddReadWriteColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->AddReadWriteColorAttachment(gbuffer.positionImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->AddReadWriteColorAttachment(gbuffer.normalImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->AddReadWriteColorAttachment(gbuffer.albedoImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->AddReadWriteColorAttachment(gbuffer.normalViewImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->AddWriteOnlyColorAttachment(distortionImage);
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

		mWaterParameterBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mEffect->BindUniformBuffer("UBO_waterParameters", &mWaterParameterBlock);

		mDuDvTexture = Vk::gTextureLoader().LoadTexture("data/textures/water_dudv.png");
		mEffect->BindCombinedImage("dudvTexture", mDuDvTexture->GetTextureDescriptorInfo());

		mQueryPool = std::make_shared<Vk::QueryPool>(mDevice);

		// const uint32_t size = 640;
		// gScreenQuadUi().AddQuad(100 + 640, 100, size, size, distortionImage.get(), renderTarget->GetSampler());
	}

	void WaterJob::Render(const JobInput& jobInput)
	{
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

		mWaterParameterBlock.data.time = Timer::Instance().GetTime();
		mWaterParameterBlock.UpdateMemory();

		renderTarget->BeginCommandBuffer("Water Tessellation pass");
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		mQueryPool->Reset(commandBuffer);

		renderTarget->BeginRenderPass();

		if (IsEnabled())
		{
			mQueryPool->Begin(commandBuffer);

			glm::mat4 world = glm::translate(glm::mat4(), glm::vec3(0.0f, jobInput.renderingSettings.waterLevel, 0.0f));
			Vk::PushConstantBlock pushConsts(world);
			commandBuffer->CmdPushConstants(mEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConsts), &pushConsts);

			commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
			commandBuffer->CmdBindDescriptorSets(mEffect);

			commandBuffer->CmdBindVertexBuffer(0, 1, mWaterMesh->GetVertxBuffer());
			commandBuffer->CmdBindIndexBuffer(mWaterMesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
			commandBuffer->CmdDrawIndexed(mWaterMesh->GetNumIndices(), 1, 0, 0, 0);

			mQueryPool->End(commandBuffer);
		}

		renderTarget->End(GetWaitSemahore(), GetCompletedSemahore());

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
