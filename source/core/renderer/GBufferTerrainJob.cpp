#include "core/renderer/GBufferTerrainJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "core/renderer/Renderer.h"
#include "vulkan/ShaderFactory.h"
#include "ImGuiRenderer.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/TextureLoader.h"
#include "ScreenQuadRenderer.h"
#include "vulkan/Vertex.h"
#include "vulkan/handles/QueryPoolStatistics.h"
#include "Camera.h"
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
		renderTarget->AddWriteOnlyColorAttachment(gbuffer.specularImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		renderTarget->AddWriteOnlyDepthAttachment(gbuffer.depthImage);
		renderTarget->SetClearColor(0, 0, 0, 1);
		renderTarget->Create();

		mQueryPool = std::make_shared<Vk::QueryPoolStatistics>(mDevice);
      	renderTarget->AddStatisticsQuery(mQueryPool);

		Vk::EffectCreateInfo effectDesc;
		effectDesc.shaderDesc.vertexShaderPath = "data/shaders/tessellation/terrain.vert";
		effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/tessellation/terrain.frag";
		effectDesc.shaderDesc.tescShaderPath = "data/shaders/tessellation/terrain.tesc";
		effectDesc.shaderDesc.teseShaderPath = "data/shaders/tessellation/terrain.tese";
		effectDesc.shaderDesc.geometryShaderPath = "data/shaders/tessellation/terrain.geom";
		effectDesc.pipelineDesc.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		effectDesc.pipelineDesc.AddTessellationState(4);
		mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, renderTarget->GetRenderPass(), effectDesc);

		mFrustumPlanesBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mSettingsBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mBrushBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		mEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
		mEffect->BindUniformBuffer("UBO_frustum", mFrustumPlanesBlock);
		mEffect->BindUniformBuffer("UBO_settings", mSettingsBlock);
		mEffect->BindUniformBuffer("UBO_brush", *mTerrain->GetBrushBlock());

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
		mEffect->BindCombinedImage("samplerHeightmap", *mTerrain->GetHeightmapImage(), *mSampler);
		mEffect->BindCombinedImage("samplerNormalmap", *mTerrain->GetNormalmapImage(), *mSampler);
		mEffect->BindCombinedImage("samplerBlendmap", *mTerrain->GetBlendmapImage(), *mSampler);

		mEffect->BindCombinedImage("samplerDiffuse", mDiffuseTextureArray);
		mEffect->BindCombinedImage("samplerNormal", mNormalTextureArray);
		mEffect->BindCombinedImage("samplerDisplacement", mDisplacementTextureArray);
	}

	void GBufferTerrainJob::Render(const JobInput& jobInput)
	{
		const Frustum& frustum = gRenderer().GetMainCamera()->GetFrustum();
		memcpy(mFrustumPlanesBlock.data.frustumPlanes, frustum.planes.data(), sizeof(glm::vec4) * 6);

		mFrustumPlanesBlock.UpdateMemory();

		mSettingsBlock.data.viewportSize = glm::vec2(mWidth, mHeight);
		mSettingsBlock.data.tessellationFactor = jobInput.renderingSettings.tessellationFactor;
		mSettingsBlock.data.edgeSize = 200.0f;
		mSettingsBlock.data.amplitude = jobInput.sceneInfo.terrain->GetAmplitudeScaling();
		mSettingsBlock.data.textureScaling = jobInput.renderingSettings.terrainTextureScaling;
		mSettingsBlock.data.bumpmapAmplitude = jobInput.renderingSettings.terrainBumpmapAmplitude;
		mSettingsBlock.data.wireframe = jobInput.renderingSettings.terrainWireframe;
		mSettingsBlock.UpdateMemory();

		renderTarget->Begin("Terrain Tessellation pass", glm::vec4(0.8f, 0.4f, 0.2f, 1.0f));
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		if (IsEnabled())
		{
			glm::mat4 world = glm::mat4();
			Vk::PushConstantBlock pushConsts(world);
			commandBuffer->CmdPushConstants(mEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConsts), &pushConsts);

			commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
			commandBuffer->CmdBindDescriptorSets(mEffect);

			Vk::Mesh* mesh = jobInput.sceneInfo.terrain->GetMesh();
			commandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
			commandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
			commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
		}

		renderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
	}

	void GBufferTerrainJob::Update()
	{
		// Display Actor creation list
		ImGuiRenderer::BeginWindow("Terrain Tessellation statistics", glm::vec2(300.0f, 10.0f), 400.0f);

		ImGuiRenderer::TextV("VS invocations: %u", mQueryPool->GetStatistics(Vk::QueryPoolStatistics::StatisticsIndex::INPUT_ASSEMBLY_VERTICES_INDEX));
		ImGuiRenderer::TextV("TC invocations: %u", mQueryPool->GetStatistics(Vk::QueryPoolStatistics::StatisticsIndex::TESSELLATION_CONTROL_SHADER_PATCHES_INDEX));
		ImGuiRenderer::TextV("TE invocations: %u", mQueryPool->GetStatistics(Vk::QueryPoolStatistics::StatisticsIndex::TESSELLATION_EVALUATION_SHADER_INVOCATIONS_INDEX));
		ImGuiRenderer::TextV("FS invocations: %u", mQueryPool->GetStatistics(Vk::QueryPoolStatistics::StatisticsIndex::FRAGMENT_SHADER_INVOCATIONS_INDEX));

		ImGuiRenderer::EndWindow();
	}
}
