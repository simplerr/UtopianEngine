#include "core/renderer/GBufferTerrainJob.h"
#include "core/renderer/SunShaftJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "core/renderer/Renderer.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/UIOverlay.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/ScreenQuadUi.h"
#include "vulkan/Vertex.h"
#include "vulkan/handles/QueryPool.h"
#include "Camera.h"
#include "Input.h"
#include <random>

namespace Utopian
{
	GBufferTerrainJob::GBufferTerrainJob(Vk::Device* device, const SharedPtr<Terrain>& terrain, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		mTerrain = terrain;
	}

	GBufferTerrainJob::~GBufferTerrainJob()
	{
	}

	void GBufferTerrainJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		renderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
		renderTarget->AddWriteOnlyColorAttachment(gbuffer.positionImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		renderTarget->AddWriteOnlyColorAttachment(gbuffer.normalImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		renderTarget->AddWriteOnlyColorAttachment(gbuffer.albedoImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		renderTarget->AddWriteOnlyColorAttachment(gbuffer.normalViewImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		renderTarget->AddWriteOnlyDepthAttachment(gbuffer.depthImage);
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/tessellation/tessellation.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/tessellation/tessellation.frag";
		shaderCreateInfo.tescShaderPath = "data/shaders/tessellation/tessellation.tesc";
		shaderCreateInfo.teseShaderPath = "data/shaders/tessellation/tessellation.tese";
		shaderCreateInfo.geometryShaderPath = "data/shaders/tessellation/tessellation.geom";
		mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, renderTarget->GetRenderPass(), shaderCreateInfo);

		//mEffect->GetPipeline()->rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		mEffect->GetPipeline()->inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		mEffect->GetPipeline()->AddTessellationState(4);

		mEffect->CreatePipeline();

		viewProjectionBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mEffect->BindUniformBuffer("UBO_viewProjection", &viewProjectionBlock);

		settingsBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mEffect->BindUniformBuffer("UBO_settings", &settingsBlock);

		brushBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mEffect->BindUniformBuffer("UBO_brush", mTerrain->GetBrushBlock().get());

		Vk::Texture* diffuseTexture = Vk::gTextureLoader().LoadTexture("data/textures/ground/Ground_17_DIF.jpg");
		Vk::Texture* normalTexture = Vk::gTextureLoader().LoadTexture("data/textures/ground/Ground_17_NRM.jpg");
		Vk::Texture* displacementTexture = Vk::gTextureLoader().LoadTexture("data/textures/ground/Ground_17_DISP.jpg");

		diffuseArray.AddTexture(diffuseTexture->imageView, renderTarget->GetSampler());
		normalArray.AddTexture(normalTexture->imageView, renderTarget->GetSampler());
		displacementArray.AddTexture(displacementTexture->imageView, renderTarget->GetSampler());

		diffuseTexture = Vk::gTextureLoader().LoadTexture("data/textures/ground/Ground_21_DIF.jpg");
		normalTexture = Vk::gTextureLoader().LoadTexture("data/textures/ground/Ground_21_NRM.jpg");
		displacementTexture = Vk::gTextureLoader().LoadTexture("data/textures/ground/Ground_21_DISP.jpg");

		diffuseArray.AddTexture(diffuseTexture->imageView, renderTarget->GetSampler());
		normalArray.AddTexture(normalTexture->imageView, renderTarget->GetSampler());
		displacementArray.AddTexture(displacementTexture->imageView, renderTarget->GetSampler());

		diffuseTexture = Vk::gTextureLoader().LoadTexture("data/textures/ground/Ground_11_DIF.jpg");
		normalTexture = Vk::gTextureLoader().LoadTexture("data/textures/ground/Ground_11_NRM.jpg");
		displacementTexture = Vk::gTextureLoader().LoadTexture("data/textures/ground/Ground_11_DISP.jpg");

		diffuseArray.AddTexture(diffuseTexture->imageView, renderTarget->GetSampler());
		normalArray.AddTexture(normalTexture->imageView, renderTarget->GetSampler());
		displacementArray.AddTexture(displacementTexture->imageView, renderTarget->GetSampler());

		sampler = std::make_shared<Vk::Sampler>(mDevice, false);
		sampler->createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler->createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler->Create();

		// Bind terrain height and normal maps
		mEffect->BindCombinedImage("samplerHeightmap", mTerrain->GetHeightmapImage().get(), sampler.get());
		mEffect->BindCombinedImage("samplerNormalmap", mTerrain->GetNormalmapImage().get(), sampler.get());
		mEffect->BindCombinedImage("samplerBlendmap", mTerrain->GetBlendmapImage().get(), sampler.get());

		mEffect->BindCombinedImage("samplerDiffuse", &diffuseArray);
		mEffect->BindCombinedImage("samplerNormal", &normalArray);
		mEffect->BindCombinedImage("samplerDisplacement", &displacementArray);

		const uint32_t size = 640;
		//gScreenQuadUi().AddQuad(size + 20, height - (size + 310), size, size, image.get(), renderTarget->GetSampler());
		gScreenQuadUi().AddQuad(0u, 0u, mWidth, mHeight, gbuffer.normalImage.get(), renderTarget->GetSampler(), 1u);

		mQueryPool = std::make_shared<Vk::QueryPool>(mDevice);
	}

	void GBufferTerrainJob::Render(const JobInput& jobInput)
	{
		viewProjectionBlock.data.view = jobInput.sceneInfo.viewMatrix;
		viewProjectionBlock.data.projection = jobInput.sceneInfo.projectionMatrix;
		viewProjectionBlock.data.time = gTimer().GetTime();

		const Frustum& frustum = gRenderer().GetMainCamera()->GetFrustum();
		memcpy(viewProjectionBlock.data.frustumPlanes, frustum.planes.data(), sizeof(glm::vec4) * 6);

		viewProjectionBlock.UpdateMemory();

		settingsBlock.data.viewportSize = glm::vec2(mWidth, mHeight);
		settingsBlock.data.tessellationFactor = jobInput.renderingSettings.tessellationFactor;
		settingsBlock.data.edgeSize = 200.0f;
		settingsBlock.data.amplitude = jobInput.sceneInfo.terrain->GetAmplitudeScaling();
		settingsBlock.data.textureScaling = jobInput.renderingSettings.terrainTextureScaling;
		settingsBlock.data.bumpmapAmplitude = jobInput.renderingSettings.terrainBumpmapAmplitude;
		settingsBlock.data.wireframe = jobInput.renderingSettings.terrainWireframe;
		settingsBlock.UpdateMemory();

		renderTarget->BeginCommandBuffer("Tessellation pass");
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

			Vk::Mesh* mesh = jobInput.sceneInfo.terrain->GetMesh();
			commandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
			commandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
			commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);

			mQueryPool->End(commandBuffer);
		}

		renderTarget->End(GetWaitSemahore(), GetCompletedSemahore());

		mQueryPool->RetreiveResults();
	}

	void GBufferTerrainJob::Update()
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
