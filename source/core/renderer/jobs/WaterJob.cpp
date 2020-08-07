#include "core/renderer/jobs/WaterJob.h"
#include "core/renderer/jobs/DeferredJob.h"
#include "core/renderer/jobs/OpaqueCopyJob.h"
#include "core/renderer/jobs/ShadowJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "core/renderer/Renderer.h"
#include "vulkan/ShaderFactory.h"
#include "core/renderer/ImGuiRenderer.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/TextureLoader.h"
#include "core/renderer/ScreenQuadRenderer.h"
#include "vulkan/Vertex.h"
#include "vulkan/handles/QueryPoolStatistics.h"
#include "core/Camera.h"
#include "core/Input.h"
#include <random>
#include <glm/gtc/matrix_transform.hpp>

namespace Utopian
{
	WaterJob::WaterJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		mWaterMesh = GeneratePatches(128.0f, 512);

		// Create sampler that returns 1.0 when sampling outside the depth image
		// Note: Duplicate from DeferredJob
		mShadowSampler = std::make_shared<Vk::Sampler>(device, false);
		mShadowSampler->createInfo.anisotropyEnable = VK_FALSE; // Anistropy filter causes artifacts at the edge between cascades
		mShadowSampler->createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		mShadowSampler->createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		mShadowSampler->createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		mShadowSampler->createInfo.borderColor = VkBorderColor::VK_BORDER_COLOR_INT_OPAQUE_WHITE;
		mShadowSampler->Create();
	}

	WaterJob::~WaterJob()
	{
		delete mWaterMesh;
	}

	void WaterJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);
		OpaqueCopyJob* opaqueCopyJob = static_cast<OpaqueCopyJob*>(jobs[JobGraph::OPAQUE_COPY_INDEX]);
		ShadowJob* shadowJob = static_cast<ShadowJob*>(jobs[JobGraph::SHADOW_INDEX]);

		distortionImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R16G16_SFLOAT, "Distortion image");

		renderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
		renderTarget->AddReadWriteColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->AddReadWriteColorAttachment(gbuffer.positionImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->AddReadWriteColorAttachment(gbuffer.normalImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->AddReadWriteColorAttachment(gbuffer.albedoImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->AddReadWriteColorAttachment(gbuffer.normalViewImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->AddWriteOnlyColorAttachment(distortionImage);
		renderTarget->AddReadWriteColorAttachment(gbuffer.specularImage);
		renderTarget->AddReadWriteDepthAttachment(gbuffer.depthImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->Create();

		mQueryPool = std::make_shared<Vk::QueryPoolStatistics>(mDevice);
		renderTarget->AddStatisticsQuery(mQueryPool);

		Vk::EffectCreateInfo effectDesc;
		effectDesc.shaderDesc.vertexShaderPath = "data/shaders/tessellation/water.vert";
		effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/tessellation/water.frag";
		effectDesc.shaderDesc.tescShaderPath = "data/shaders/tessellation/water.tesc";
		effectDesc.shaderDesc.teseShaderPath = "data/shaders/tessellation/water.tese";
		effectDesc.shaderDesc.geometryShaderPath = "data/shaders/tessellation/water.geom";
		effectDesc.pipelineDesc.blendingType = Vk::BlendingType::BLENDING_ALPHA;
		effectDesc.pipelineDesc.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		effectDesc.pipelineDesc.AddTessellationState(4);

		mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, renderTarget->GetRenderPass(), effectDesc);

		mFrustumPlanesBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mSettingsBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mWaterParameterBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mLightBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mCascadeBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		mEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
		mEffect->BindUniformBuffer("UBO_frustum", mFrustumPlanesBlock);
		mEffect->BindUniformBuffer("UBO_settings", mSettingsBlock);
		mEffect->BindUniformBuffer("UBO_waterParameters", mWaterParameterBlock);
		mEffect->BindUniformBuffer("UBO_lights", mLightBlock);
		mEffect->BindUniformBuffer("UBO_cascades", mCascadeBlock);

		mDuDvTexture = Vk::gTextureLoader().LoadTexture("data/textures/water_dudv.png");
		mNormalTexture = Vk::gTextureLoader().LoadTexture("data/textures/water_normal.png");
		mFoamMaskTexture = Vk::gTextureLoader().LoadTexture("data/textures/water_foam.png");

		mEffect->BindCombinedImage("dudvSampler", *mDuDvTexture);
		mEffect->BindCombinedImage("normalSampler", *mNormalTexture);
		mEffect->BindCombinedImage("foamMaskSampler", *mFoamMaskTexture);
		mEffect->BindCombinedImage("depthSampler", *opaqueCopyJob->opaqueDepthImage, *renderTarget->GetSampler());
		mEffect->BindCombinedImage("shadowSampler", *shadowJob->depthColorImage, *mShadowSampler);

		// const uint32_t size = 640;
		// gScreenQuadUi().AddQuad(100 + 640, 100, size, size, distortionImage.get(), renderTarget->GetSampler());
	}

	void WaterJob::Render(const JobInput& jobInput)
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

		mWaterParameterBlock.data.waterColor = jobInput.renderingSettings.waterColor;
		mWaterParameterBlock.data.foamColor = jobInput.renderingSettings.foamColor;
		mWaterParameterBlock.data.waveSpeed = jobInput.renderingSettings.waveSpeed;
		mWaterParameterBlock.data.foamSpeed = jobInput.renderingSettings.foamSpeed;
		mWaterParameterBlock.data.distortionStrength = jobInput.renderingSettings.waterDistortionStrength;
		mWaterParameterBlock.data.shorelineDepth = jobInput.renderingSettings.shorelineDepth;
		mWaterParameterBlock.data.waveFrequency = jobInput.renderingSettings.waveFrequency;
		mWaterParameterBlock.data.waterSpecularity = jobInput.renderingSettings.waterSpecularity;
		mWaterParameterBlock.UpdateMemory();

		// Note: Todo: Temporary
		// Note: Duplicate from DeferredJob.cpp
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			mCascadeBlock.data.cascadeSplits[i] = jobInput.sceneInfo.cascades[i].splitDepth;
			mCascadeBlock.data.cascadeViewProjMat[i] = jobInput.sceneInfo.cascades[i].viewProjMatrix;
		}

		// Note: This should probably be moved. We need the fragment position in view space
		// when comparing it's Z value to find out which shadow map cascade it should sample from.
		mCascadeBlock.data.cameraViewMat = jobInput.sceneInfo.viewMatrix;
		mCascadeBlock.data.shadowSampleSize = jobInput.renderingSettings.shadowSampleSize;
		mCascadeBlock.data.shadowsEnabled = jobInput.renderingSettings.shadowsEnabled;
		mCascadeBlock.UpdateMemory();

		// Upload lights to shader. Todo: Duplicate of SetLightArray() in DeferredEffect.cpp
		mLightBlock.lights.clear();
		for (auto& light : jobInput.sceneInfo.lights)
		{
			mLightBlock.lights.push_back(light->GetLightData());
		}

		mLightBlock.constants.numLights = mLightBlock.lights.size();
		mLightBlock.UpdateMemory();

		renderTarget->Begin("Water Tessellation pass", glm::vec4(0.0f, 0.7f, 0.7f, 1.0f));
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		if (IsEnabled())
		{
			glm::mat4 world = glm::translate(glm::mat4(), glm::vec3(0.0f, jobInput.renderingSettings.waterLevel, 0.0f));
			Vk::PushConstantBlock pushConsts(world);
			commandBuffer->CmdPushConstants(mEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConsts), &pushConsts);

			commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
			commandBuffer->CmdBindDescriptorSets(mEffect);

			commandBuffer->CmdBindVertexBuffer(0, 1, mWaterMesh->GetVertxBuffer());
			commandBuffer->CmdBindIndexBuffer(mWaterMesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
			commandBuffer->CmdDrawIndexed(mWaterMesh->GetNumIndices(), 1, 0, 0, 0);
		}

		renderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
	}

	void WaterJob::Update()
	{
		// Display Actor creation list
		ImGuiRenderer::BeginWindow("Water Tessellation statistics", glm::vec2(300.0f, 10.0f), 400.0f);

		ImGuiRenderer::TextV("VS invocations: %u", mQueryPool->GetStatistics(Vk::QueryPoolStatistics::StatisticsIndex::INPUT_ASSEMBLY_VERTICES_INDEX));
		ImGuiRenderer::TextV("TC invocations: %u", mQueryPool->GetStatistics(Vk::QueryPoolStatistics::StatisticsIndex::TESSELLATION_CONTROL_SHADER_PATCHES_INDEX));
		ImGuiRenderer::TextV("TE invocations: %u", mQueryPool->GetStatistics(Vk::QueryPoolStatistics::StatisticsIndex::TESSELLATION_EVALUATION_SHADER_INVOCATIONS_INDEX));
		ImGuiRenderer::TextV("FS invocations: %u", mQueryPool->GetStatistics(Vk::QueryPoolStatistics::StatisticsIndex::FRAGMENT_SHADER_INVOCATIONS_INDEX));

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

		mesh->SetDebugName("Water patches");
		mesh->BuildBuffers(mDevice);

		return mesh;
	}
}
