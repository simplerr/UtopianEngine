#include "core/renderer/GBufferTerrainJob.h"
#include "core/renderer/SunShaftJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "core/renderer/Renderer.h"
#include "vulkan/ShaderFactory.h"
#include "ImGuiRenderer.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/TextureLoader.h"
#include "ScreenQuadRenderer.h"
#include "vulkan/Vertex.h"
#include "vulkan/Texture2.h"
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
		renderTarget->SetClearColor(0, 0, 0, 1);
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

		mViewProjectionBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mEffect->BindUniformBuffer("UBO_viewProjection", &mViewProjectionBlock);

		mSettingsBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mEffect->BindUniformBuffer("UBO_settings", &mSettingsBlock);

		mBrushBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mEffect->BindUniformBuffer("UBO_brush", mTerrain->GetBrushBlock().get());

		TerrainMaterial material = mTerrain->GetMaterial("grass");
		mDiffuseTextureArray.AddTexture(material.diffuse);
		mNormalTextureArray.AddTexture(material.normal);
		mDisplacementTextureArray.AddTexture(material.displacement);

		material = mTerrain->GetMaterial("rock");
		mDiffuseTextureArray.AddTexture(material.diffuse);
		mNormalTextureArray.AddTexture(material.normal);
		mDisplacementTextureArray.AddTexture(material.displacement);

		material = mTerrain->GetMaterial("dirt");
		mDiffuseTextureArray.AddTexture(material.diffuse);
		mNormalTextureArray.AddTexture(material.normal);
		mDisplacementTextureArray.AddTexture(material.displacement);

		mSampler = std::make_shared<Vk::Sampler>(mDevice, false);
		mSampler->createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		mSampler->createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		mSampler->Create();

		// Bind terrain height and normal maps
		mEffect->BindCombinedImage("samplerHeightmap", mTerrain->GetHeightmapImage().get(), mSampler.get());
		mEffect->BindCombinedImage("samplerNormalmap", mTerrain->GetNormalmapImage().get(), mSampler.get());
		mEffect->BindCombinedImage("samplerBlendmap", mTerrain->GetBlendmapImage().get(), mSampler.get());

		mEffect->BindCombinedImage("samplerDiffuse", &mDiffuseTextureArray);
		mEffect->BindCombinedImage("samplerNormal", &mNormalTextureArray);
		mEffect->BindCombinedImage("samplerDisplacement", &mDisplacementTextureArray);

		mQueryPool = std::make_shared<Vk::QueryPool>(mDevice);
	}

	void GBufferTerrainJob::Render(const JobInput& jobInput)
	{
		mViewProjectionBlock.data.view = jobInput.sceneInfo.viewMatrix;
		mViewProjectionBlock.data.projection = jobInput.sceneInfo.projectionMatrix;
		mViewProjectionBlock.data.time = gTimer().GetTime();

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
		ImGuiRenderer::BeginWindow("Tessellation statistics", glm::vec2(300.0f, 10.0f), 400.0f);

		ImGuiRenderer::TextV("VS invocations: %u", mQueryPool->GetStatistics(Vk::QueryPool::StatisticsIndex::INPUT_ASSEMBLY_VERTICES_INDEX));
		ImGuiRenderer::TextV("TC invocations: %u", mQueryPool->GetStatistics(Vk::QueryPool::StatisticsIndex::TESSELLATION_CONTROL_SHADER_PATCHES_INDEX));
		ImGuiRenderer::TextV("TE invocations: %u", mQueryPool->GetStatistics(Vk::QueryPool::StatisticsIndex::TESSELLATION_EVALUATION_SHADER_INVOCATIONS_INDEX));
		ImGuiRenderer::TextV("FS invocations: %u", mQueryPool->GetStatistics(Vk::QueryPool::StatisticsIndex::FRAGMENT_SHADER_INVOCATIONS_INDEX));

		ImGuiRenderer::EndWindow();
	}
}
