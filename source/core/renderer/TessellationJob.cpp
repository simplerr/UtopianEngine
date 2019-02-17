#include "core/renderer/TessellationJob.h"
#include "core/renderer/SunShaftJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/UIOverlay.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/ScreenQuadUi.h"
#include "vulkan/Vertex.h"
#include "vulkan/handles/QueryPool.h"
#include <random>

namespace Utopian
{
	TessellationJob::TessellationJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		image = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT);
		depthImage = std::make_shared<Vk::ImageDepth>(device, width, height, VK_FORMAT_D32_SFLOAT_S8_UINT);

		renderTarget = std::make_shared<Vk::RenderTarget>(device, width, height);
		renderTarget->AddColorAttachment(image);
		renderTarget->AddDepthAttachment(depthImage);
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/tessellation/tessellation.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/tessellation/tessellation.frag";
		shaderCreateInfo.tescShaderPath = "data/shaders/tessellation/tessellation.tesc";
		shaderCreateInfo.teseShaderPath = "data/shaders/tessellation/tessellation.tese";
		mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(device, renderTarget->GetRenderPass(), shaderCreateInfo);

		//mEffect->GetPipeline()->rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		mEffect->GetPipeline()->inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		mEffect->GetPipeline()->AddTessellationState(4);

		mEffect->CreatePipeline();

		viewProjectionBlock.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mEffect->BindUniformBuffer("UBO_viewProjection", &viewProjectionBlock);

		settingsBlock.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mEffect->BindUniformBuffer("UBO_settings", &settingsBlock);

		mQueryPool = std::make_shared<Vk::QueryPool>(device);

		const uint32_t size = 640;
		//gScreenQuadUi().AddQuad(size + 20, height - (size + 310), size, size, image.get(), renderTarget->GetSampler());
		gScreenQuadUi().AddQuad(0u, 0u, width, height, image.get(), renderTarget->GetSampler(), 1u);
	}

	TessellationJob::~TessellationJob()
	{
	}

	void TessellationJob::Init(const std::vector<BaseJob*>& renderers)
	{
		SunShaftJob* sunShaftJob = static_cast<SunShaftJob*>(renderers[JobGraph::SUN_SHAFT_INDEX]);
		SetWaitSemaphore(sunShaftJob->GetCompletedSemahore());
	
		GeneratePatches(512.0f, 128);
		GenerateTerrainMaps();
	}

	void TessellationJob::GenerateTerrainMaps()
	{
		uint32_t resolution = 2048;
		/* Height map */
		heightmapImage = std::make_shared<Vk::ImageColor>(mDevice, resolution, resolution, VK_FORMAT_R16G16B16A16_SFLOAT);

		heightmapRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, resolution, resolution);
		heightmapRenderTarget->AddColorAttachment(heightmapImage);
		heightmapRenderTarget->SetClearColor(1, 1, 1, 1);
		heightmapRenderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/tessellation/heightmap.frag";
		mHeightmapEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, heightmapRenderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		mHeightmapEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mHeightmapEffect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		mHeightmapEffect->CreatePipeline();

		/* Normal map */
		normalImage = std::make_shared<Vk::ImageColor>(mDevice, resolution, resolution, VK_FORMAT_R16G16B16A16_SFLOAT);

		normalRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, resolution, resolution);
		normalRenderTarget->AddColorAttachment(normalImage);
		normalRenderTarget->SetClearColor(1, 1, 1, 1);
		normalRenderTarget->Create();

		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/tessellation/normalmap.frag";
		mNormalmapEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, normalRenderTarget->GetRenderPass(), shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		mNormalmapEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mNormalmapEffect->GetPipeline()->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		mNormalmapEffect->CreatePipeline();

		mNormalmapEffect->BindCombinedImage("samplerHeightmap", heightmapImage.get(), heightmapRenderTarget->GetSampler());

		/* Render the heightmap to texture */
		heightmapRenderTarget->Begin("Heightmap pass", glm::vec4(0.5, 1.0, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = heightmapRenderTarget->GetCommandBuffer();
		commandBuffer->CmdBindPipeline(mHeightmapEffect->GetPipeline());
		gRendererUtility().DrawFullscreenQuad(commandBuffer);
		heightmapRenderTarget->End();

		/* Render the heightmap to texture */
		normalRenderTarget->Begin("Normalmap pass", glm::vec4(0.5, 1.0, 0.0, 1.0));
		commandBuffer = normalRenderTarget->GetCommandBuffer();
		commandBuffer->CmdBindPipeline(mNormalmapEffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(mNormalmapEffect);
		gRendererUtility().DrawFullscreenQuad(commandBuffer);
		normalRenderTarget->End();

		// Bind terrain height and normal maps
		mEffect->BindCombinedImage("samplerHeightmap", heightmapImage.get(), heightmapRenderTarget->GetSampler());
		mEffect->BindCombinedImage("samplerNormalmap", normalImage.get(), normalRenderTarget->GetSampler());

		//const uint32_t size = 640;
		//gScreenQuadUi().AddQuad(300 + 20, mHeight - (size + 310), size, size, heightmapImage.get(), heightmapRenderTarget->GetSampler());
		//gScreenQuadUi().AddQuad(300 + size + 20, mHeight - (size + 310), size, size, normalImage.get(), normalRenderTarget->GetSampler());
	}

	void TessellationJob::GeneratePatches(float cellSize, int numCells)
	{
		mQuadModel = new Vk::StaticModel();
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
				vertex.Tex = glm::vec2((float)x / numCells, (float)z / numCells);
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

		mesh->LoadTextures("data/textures/ground/grass2.tga");
		mesh->BuildBuffers(mDevice);
		mQuadModel->AddMesh(mesh);
	}

	void TessellationJob::Render(const JobInput& jobInput)
	{
		viewProjectionBlock.data.view = jobInput.sceneInfo.viewMatrix;
		viewProjectionBlock.data.projection = jobInput.sceneInfo.projectionMatrix;
		viewProjectionBlock.UpdateMemory();

		settingsBlock.data.viewportSize = glm::vec2(mWidth, mHeight);
		settingsBlock.data.tessellationFactor = jobInput.renderingSettings.tessellationFactor;
		settingsBlock.data.edgeSize = 200.0f;
		settingsBlock.data.amplitude = jobInput.renderingSettings.terrainAmplitude;
		settingsBlock.data.textureScaling = jobInput.renderingSettings.terrainTextureScaling;
		settingsBlock.UpdateMemory();

		renderTarget->BeginCommandBuffer("Tessellation pass", glm::vec4(0.0, 1.0, 0.0, 1.0));
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

			Vk::Mesh* mesh = mQuadModel->mMeshes[0];
			commandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
			commandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
			commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);

			mQueryPool->End(commandBuffer);
		}

		renderTarget->End(GetWaitSemahore(), GetCompletedSemahore());

		mQueryPool->RetreiveResults();
	}

	void TessellationJob::Update()
	{
		// Display Actor creation list
		Vk::UIOverlay::BeginWindow("Tessellation statistics", glm::vec2(300.0f, 10.0f), 400.0f);

		Vk::UIOverlay::TextV("VS invocations: %u", mQueryPool->GetStatistics(Vk::QueryPool::StatisticsIndex::INPUT_ASSEMBLY_VERTICES_INDEX));
		Vk::UIOverlay::TextV("TC invocations: %u", mQueryPool->GetStatistics(Vk::QueryPool::StatisticsIndex::TESSELLATION_CONTROL_SHADER_PATCHES_INDEX));
		Vk::UIOverlay::TextV("TE invocations: %u", mQueryPool->GetStatistics(Vk::QueryPool::StatisticsIndex::TESSELLATION_EVALUATION_SHADER_INVOCATIONS_INDEX));
		Vk::UIOverlay::TextV("FS invocations: %u", mQueryPool->GetStatistics(Vk::QueryPool::StatisticsIndex::FRAGMENT_SHADER_INVOCATIONS_INDEX));

		Vk::UIOverlay::EndWindow();
	}
}
