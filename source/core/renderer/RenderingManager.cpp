#include <glm/gtc/matrix_transform.hpp>
#include "core/renderer/RenderingManager.h"
#include "core/renderer/Renderable.h"
#include "vulkan/Renderer.h"
#include "vulkan/Vertex.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/Image.h"
#include "Camera.h"
#include "vulkan/Mesh.h"
#include "vulkan/StaticModel.h"
#include "core/renderer/Light.h"
#include "core/terrain/Terrain.h"
#include "WaterRenderer.h"
#include "vulkan/ScreenQuadUi.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/BasicRenderTarget.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/UIOverlay.h"
#include "core/terrain/PerlinTerrain.h"
#include "core/AssetLoader.h"
#include "core/renderer/GBufferJob.h"
#include "core/renderer/SSAOJob.h"
#include "core/renderer/BlurJob.h"
#include "core/renderer/ShadowJob.h"
#include "core/renderer/DeferredJob.h"
#include "core/renderer/GrassJob.h"
#include "core/renderer/SkydomeJob.h"
#include "core/renderer/SunShaftJob.h"
#include "core/renderer/DebugJob.h"
#include "vulkan/ScreenQuadUi.h"

namespace Utopian
{
	RenderingManager& gRenderingManager()
	{
		return RenderingManager::Instance();
	}

	RenderingManager::RenderingManager(Vk::Renderer* renderer)
	{
		mNextNodeId = 0;
		mMainCamera = nullptr;
		mMainCamera = renderer->GetCamera();
		mRenderer = renderer;
		mCommandBuffer = mRenderer->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

		// To solve problem with RenderDoc not working with a secondary command buffer that is empty.
		// The real solution is to cleanup and remove this.
		mCommandBuffer->ToggleActive();

		mWaterRenderer = new WaterRenderer(mRenderer, &Vk::gTextureLoader());
		mWaterRenderer->AddWater(glm::vec3(123000.0f, 0.0f, 106000.0f), 20);
		mWaterRenderer->AddWater(glm::vec3(103000.0f, 0.0f, 96000.0f), 20);

		AddJob(new GBufferJob(renderer, renderer->GetWindowWidth(), renderer->GetWindowHeight()));
		AddJob(new SSAOJob(renderer, renderer->GetWindowWidth(), renderer->GetWindowHeight()));
		AddJob(new BlurJob(renderer, renderer->GetWindowWidth(), renderer->GetWindowHeight()));
		AddJob(new ShadowJob(renderer, renderer->GetWindowWidth(), renderer->GetWindowHeight()));
		AddJob(new DeferredJob(renderer, renderer->GetWindowWidth(), renderer->GetWindowHeight()));
		AddJob(new GrassJob(renderer, renderer->GetWindowWidth(), renderer->GetWindowHeight()));
		//AddJob(new SkyboxJob(renderer, renderer->GetWindowWidth(), renderer->GetWindowHeight()));
		AddJob(new SkydomeJob(renderer, renderer->GetWindowWidth(), renderer->GetWindowHeight()));
		AddJob(new SunShaftJob(renderer, renderer->GetWindowWidth(), renderer->GetWindowHeight()));
		AddJob(new DebugJob(renderer, renderer->GetWindowWidth(), renderer->GetWindowHeight()));

		// Default rendering settings
		mRenderingSettings.deferredPipeline = true;
		mRenderingSettings.fogColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
		mRenderingSettings.fogStart = 40000.0f;
		mRenderingSettings.fogDistance = 16000.0f;

		mSceneInfo.directionalLight = nullptr;
	}

	RenderingManager::~RenderingManager()
	{
	}

	void RenderingManager::PostWorldInit()
	{
		InitShaderResources();
		InitShader();

		mSceneInfo.terrain = std::make_shared<PerlinTerrain>(mRenderer);
		//mSceneInfo.terrain->SetViewDistance(4);
	}

	void RenderingManager::InitShaderResources()
	{
		for (auto& light : mSceneInfo.lights)
		{
			per_frame_ps.lights.push_back(light->GetLightData());
		}

		per_frame_ps.constants.numLights = per_frame_ps.lights.size();

		per_frame_vs.Create(mRenderer->GetDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		per_frame_ps.Create(mRenderer->GetDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		fog_ubo.Create(mRenderer->GetDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		mCommonDescriptorPool = new Vk::DescriptorPool(mRenderer->GetDevice());
		mCommonDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100);
		mCommonDescriptorPool->Create();

		mCommonDescriptorSetLayout.AddUniformBuffer(0, VK_SHADER_STAGE_VERTEX_BIT);
		mCommonDescriptorSetLayout.AddUniformBuffer(1, VK_SHADER_STAGE_FRAGMENT_BIT);
		mCommonDescriptorSetLayout.AddUniformBuffer(2, VK_SHADER_STAGE_FRAGMENT_BIT);
		mCommonDescriptorSetLayout.Create(mRenderer->GetDevice());

		mCommonDescriptorSet = new Vk::DescriptorSet(mRenderer->GetDevice(), &mCommonDescriptorSetLayout, mCommonDescriptorPool);
		mCommonDescriptorSet->BindUniformBuffer(0, per_frame_vs.GetDescriptor());
		mCommonDescriptorSet->BindUniformBuffer(1, per_frame_ps.GetDescriptor());
		mCommonDescriptorSet->BindUniformBuffer(2, fog_ubo.GetDescriptor());
		mCommonDescriptorSet->UpdateDescriptorSets();

		mEffects[Vk::EffectType::PHONG] = new Vk::PhongEffect();
		mEffects[Vk::EffectType::PHONG]->Init(mRenderer);
	}

	void RenderingManager::InitShader()
	{
		mPhongEffect.Init(mRenderer);
	}

	void RenderingManager::Update()
	{
		UpdateCascades();

		mSceneInfo.terrain->Update();

		per_frame_ps.lights.clear();
		for (auto& light : mSceneInfo.lights)
		{
			per_frame_ps.lights.push_back(light->GetLightData());
		}

		mCommonDescriptorSet->BindUniformBuffer(0, per_frame_vs.GetDescriptor());
		mCommonDescriptorSet->BindUniformBuffer(1, per_frame_ps.GetDescriptor());
		mCommonDescriptorSet->BindUniformBuffer(2, fog_ubo.GetDescriptor());
		mCommonDescriptorSet->UpdateDescriptorSets();
		mTerrain->Update();
		mWaterRenderer->Update(mRenderer, mMainCamera);
	
		UpdateUi();
	}
		
	void RenderingManager::UpdateUi()
	{
		// Draw UI overlay for rendering settings
		// It's expected that each rendering node might have it's own settings that can be configured 
		Vk::UIOverlay::BeginWindow("Rendering settings", glm::vec2(10, 150), 300.0f);
		static bool debugQuads = false;

		//ImGui::Checkbox("Deferred pipeline", &mRenderingSettings.deferredPipeline);
		ImGui::Checkbox("Debug quads", &debugQuads);
		//ImGui::ColorEdit4("Fog color", &mRenderingSettings.fogColor.x);
		//ImGui::SliderFloat("Fog start", &mRenderingSettings.fogStart, 0.0f, 100000.0f);
		//ImGui::SliderFloat("Fog distance", &mRenderingSettings.fogDistance, 0.0f, 100000.0f);
		ImGui::SliderFloat("SSAO radius", &mRenderingSettings.ssaoRadius, 0.0f, 20.0f);
		ImGui::SliderFloat("SSAO bias", &mRenderingSettings.ssaoBias, 0.0f, 10.0f);
		ImGui::SliderInt("SSAO blur radius", &mRenderingSettings.blurRadius, 1, 20);
		ImGui::SliderInt("Block view distance", &mRenderingSettings.blockViewDistance, 1, 10);
		ImGui::SliderFloat("Grass view distance", &mRenderingSettings.grassViewDistance, 0.0f, 10000.0f);
		ImGui::Checkbox("Shadows", &mRenderingSettings.shadowsEnabled);
		ImGui::Checkbox("Normal mapping", &mRenderingSettings.normalMapping);
		ImGui::Checkbox("SSAO", &mRenderingSettings.ssaoEnabled);
		ImGui::SliderInt("Shadow sample size", &mRenderingSettings.shadowSampleSize, 0, 10);
		ImGui::Checkbox("Cascade color debug", &mRenderingSettings.cascadeColorDebug);
		ImGui::SliderFloat("Cascade split lambda", &mRenderingSettings.cascadeSplitLambda, 0.0f, 1.0f);
		ImGui::SliderFloat("Near plane", &mRenderingSettings.nearPlane, 0.0f, 100.0f);
		ImGui::SliderFloat("Far plane", &mRenderingSettings.farPlane, 1000.0f, 25600.0f);
		ImGui::SliderFloat("Sun inclination", &mRenderingSettings.sunInclination, -90.0f, 90.0f);
		//ImGui::SliderFloat("Sun azimuth", &mRenderingSettings.sunAzimuth, -180.0f, 180.0f);
		ImGui::SliderFloat("Sun speed", &mRenderingSettings.sunSpeed, 0.0f, 10.0f);

		// Temp:
		mMainCamera->SetNearPlane(mRenderingSettings.nearPlane);
		mMainCamera->SetFarPlane(mRenderingSettings.farPlane);
		gScreenQuadUi().SetVisible(0, debugQuads);

		Vk::UIOverlay::EndWindow();

		Vk::UIOverlay::BeginWindow("Utopian v0.1", glm::vec2(10, 10), 350.0f);

		glm::vec3 pos = mMainCamera->GetPosition();
		glm::vec3 dir = mMainCamera->GetDirection();

		Vk::UIOverlay::TextV("Time: %.2f", Timer::Instance().GetTime());
		Vk::UIOverlay::TextV("Camera pos = (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
		Vk::UIOverlay::TextV("Camera dir = (%.2f, %.2f, %.2f)", dir.x, dir.y, dir.z);
		Vk::UIOverlay::TextV("Models: %u, Lights: %u", mSceneInfo.renderables.size(), mSceneInfo.lights.size());
		Vk::UIOverlay::TextV("Blocks %u", mSceneInfo.terrain->GetNumBlocks());

		Vk::UIOverlay::EndWindow();
	}

	/*
		Calculate frustum split depths and matrices for the shadow map cascades
		Based on https://johanmedestrom.wordpress.com/2016/03/18/opengl-cascaded-shadow-maps/
		From Sascha Willems example demos.
	*/
	void RenderingManager::UpdateCascades()
	{
		float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];

		float nearClip = mMainCamera->GetNearPlane();
		float farClip = mMainCamera->GetFarPlane();
		float clipRange = farClip - nearClip;

		float minZ = nearClip;
		float maxZ = nearClip + clipRange;

		float range = maxZ - minZ;
		float ratio = maxZ / minZ;

		// Calculate split depths based on view camera furstum
		// Based on method presentd in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			float p = (i + 1) / static_cast<float>(SHADOW_MAP_CASCADE_COUNT);
			float log = minZ * std::pow(ratio, p);
			float uniform = minZ + range * p;
			float d = mRenderingSettings.cascadeSplitLambda * (log - uniform) + uniform;
			cascadeSplits[i] = (d - nearClip) / clipRange;
		}

		// Calculate orthographic projection matrix for each cascade
		float lastSplitDist = 0.0;
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			float splitDist = cascadeSplits[i];

			glm::vec3 frustumCorners[8] = {
				glm::vec3(-1.0f,  1.0f, -1.0f),
				glm::vec3(1.0f,  1.0f, -1.0f),
				glm::vec3(1.0f, -1.0f, -1.0f),
				glm::vec3(-1.0f, -1.0f, -1.0f),
				glm::vec3(-1.0f,  1.0f,  1.0f),
				glm::vec3(1.0f,  1.0f,  1.0f),
				glm::vec3(1.0f, -1.0f,  1.0f),
				glm::vec3(-1.0f, -1.0f,  1.0f),
			};

			// Project frustum corners into world space
			glm::mat4 invCam = glm::inverse(mMainCamera->GetProjection() * mMainCamera->GetView());
			for (uint32_t i = 0; i < 8; i++) {
				glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
				frustumCorners[i] = invCorner / invCorner.w;
			}

			for (uint32_t i = 0; i < 4; i++) {
				glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
				frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
				frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
			}

			// Get frustum center
			glm::vec3 frustumCenter = glm::vec3(0.0f);
			for (uint32_t i = 0; i < 8; i++) {
				frustumCenter += frustumCorners[i];
			}
			frustumCenter /= 8.0f;

			float radius = 0.0f;
			for (uint32_t i = 0; i < 8; i++) {
				float distance = glm::length(frustumCorners[i] - frustumCenter);
				radius = glm::max(radius, distance);
			}
			radius = std::ceil(radius * 16.0f) / 16.0f;

			glm::vec3 maxExtents = glm::vec3(radius);
			glm::vec3 minExtents = -maxExtents;

			// Todo: Note: vec3(-1, 1, -1) is needed to make the shadows match phong shading
			glm::vec3 lightDir = glm::vec3(-1, 1, -1) * glm::normalize(mSceneInfo.directionalLight->GetDirection());
			glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));

			// glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);
			// Note: from Saschas examples the zNear was 0.0f, unclear why I need to set it to -(maxExtents.z - minExtents.z).
			glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, -(maxExtents.z - minExtents.z), maxExtents.z - minExtents.z);

			// Store split distance and matrix in cascade
			mSceneInfo.cascades[i].splitDepth = (mMainCamera->GetNearPlane() + splitDist * clipRange) * -1.0f;
			mSceneInfo.cascades[i].viewProjMatrix = lightOrthoMatrix * lightViewMatrix;

			lastSplitDist = cascadeSplits[i];
		}
	}

	void RenderingManager::RenderNodes(Vk::CommandBuffer* commandBuffer)
	{
		for (auto& renderable : mSceneInfo.renderables)
		{
			Vk::StaticModel* model = renderable->GetModel();

			// Todo:: should be able to use other pipelines that PhongEffect

			for (Vk::Mesh* mesh : model->mMeshes)
			{
				// Push the world matrix constant
				Vk::PushConstantBlock pushConsts(renderable->GetTransform().GetWorldMatrix(), renderable->GetColor());

				commandBuffer->CmdBindPipeline(mEffects[renderable->GetMaterial().effectType]->GetPipeline(renderable->GetMaterial().variation));

				VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptorSet();
				VkDescriptorSet descriptorSets[2] = { mCommonDescriptorSet->descriptorSet, textureDescriptorSet };
				vkCmdBindDescriptorSets(commandBuffer->GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mEffects[Vk::EffectType::PHONG]->GetPipelineLayout(), 0, 2, descriptorSets, 0, NULL);

				commandBuffer->CmdPushConstants(mEffects[Vk::EffectType::PHONG], VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConsts), &pushConsts);
				commandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
				commandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
				commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
			}
		}
	}

	void RenderingManager::RenderScene(Vk::CommandBuffer* commandBuffer)
	{
		UpdateUniformBuffers();

		mTerrain->Render(commandBuffer, mCommonDescriptorSet);
		RenderNodes(commandBuffer);
	}

	void RenderingManager::Render()
	{
		if (mRenderingSettings.deferredPipeline == true)
		{
			/* Deferred rendering pipeline */
			mSceneInfo.viewMatrix = mMainCamera->GetView();
			mSceneInfo.projectionMatrix = mMainCamera->GetProjection();
			mSceneInfo.eyePos = mMainCamera->GetPosition();

			JobInput jobInput(mSceneInfo, mJobs, mRenderingSettings);
			for (auto& job : mJobs)
			{
				job->Render(mRenderer, jobInput);
			}
		}
		else
		{
			/* Legacy forward rendering pipeline */
			RenderOffscreen();

			mCommandBuffer->Begin(mRenderer->GetRenderPass(), mRenderer->GetCurrentFrameBuffer());
			mCommandBuffer->CmdSetViewPort(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());
			mCommandBuffer->CmdSetScissor(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());

			RenderScene(mCommandBuffer);

			mWaterRenderer->Render(mRenderer, mCommandBuffer);

			mCommandBuffer->End();
		}
	}

	void RenderingManager::RenderOffscreen()
	{
		glm::vec3 cameraPos = mMainCamera->GetPosition();

		// Reflection renderpass
		mMainCamera->SetPosition(mMainCamera->GetPosition() - glm::vec3(0, mMainCamera->GetPosition().y *  2, 0)); // NOTE: Water is hardcoded to be at y = 0
		mMainCamera->SetOrientation(mMainCamera->GetYaw(), -mMainCamera->GetPitch());
		SetClippingPlane(glm::vec4(0, -1, 0, 0));

		mWaterRenderer->GetReflectionRenderTarget()->Begin();	
		RenderScene(mWaterRenderer->GetReflectionRenderTarget()->GetCommandBuffer());
		mWaterRenderer->GetReflectionRenderTarget()->End(mRenderer->GetQueue());

		// Refraction renderpass
		SetClippingPlane(glm::vec4(0, 1, 0, 0));
		mMainCamera->SetPosition(cameraPos);
		mMainCamera->SetOrientation(mMainCamera->GetYaw(), -mMainCamera->GetPitch());

		mWaterRenderer->GetRefractionRenderTarget()->Begin();	
		RenderScene(mWaterRenderer->GetRefractionRenderTarget()->GetCommandBuffer());
		mWaterRenderer->GetRefractionRenderTarget()->End(mRenderer->GetQueue());

		SetClippingPlane(glm::vec4(0, 1, 0, 1500000));

		UpdateUniformBuffers();
	}

	void RenderingManager::UpdateUniformBuffers()
	{
		// From Renderer.cpp
		if (mMainCamera != nullptr)
		{
			per_frame_vs.camera.projectionMatrix = mMainCamera->GetProjection();
			per_frame_vs.camera.viewMatrix = mMainCamera->GetView();
			per_frame_vs.camera.clippingPlane = mClippingPlane;
			per_frame_vs.camera.eyePos = mMainCamera->GetPosition();
		}

		fog_ubo.data.fogColor = mRenderer->GetClearColor();
		fog_ubo.data.fogStart = 41500.0f;
		fog_ubo.data.fogDistance = 15400.0f;

		per_frame_vs.UpdateMemory();
		per_frame_ps.UpdateMemory();
		fog_ubo.UpdateMemory();
	}

	void RenderingManager::AddRenderable(Renderable* renderable)
	{
		renderable->SetId(mNextNodeId++);
		mSceneInfo.renderables.push_back(renderable);
	}

	void RenderingManager::AddLight(Light* light)
	{
		if (light->GetType() == 0) { // Directional
			if (mSceneInfo.directionalLight == nullptr)
				mSceneInfo.directionalLight = light;
			else
				assert(0); // Only one directional light in the scene
		}

		light->SetId(mNextNodeId++);
		mSceneInfo.lights.push_back(light);
	}

	void RenderingManager::AddCamera(Camera* camera)
	{
		camera->SetId(mNextNodeId++);
		mSceneInfo.cameras.push_back(camera);
	}

	InstanceGroup::InstanceGroup(uint32_t assetId)
	{
		mAssetId = assetId;
		mInstanceBuffer = nullptr;

		mModel = gAssetLoader().LoadAsset(assetId);

		assert(mModel);
	}
		
	void InstanceGroup::AddInstance(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
	{
		glm::mat4 world = glm::translate(glm::mat4(), position);
		world = glm::rotate(world, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		world = glm::rotate(world, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		world = glm::rotate(world, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		world = glm::scale(world, scale);

		InstanceData instanceData;
		instanceData.world = world;

		mInstances.push_back(instanceData);
	}

	void InstanceGroup::ClearInstances()
	{
		mInstances.clear();
		mInstanceBuffer = nullptr;
	}

	void InstanceGroup::BuildBuffer(Vk::Renderer* renderer)
	{
		// Todo: use device local buffer for better performance
		// Note: Recreating the buffer every time since if the size has increased just
		// mapping and updating the memory is not enough.
		if (GetNumInstances() != 0)
		{
			mInstanceBuffer = std::make_shared<Vk::Buffer>(renderer->GetDevice(),
														   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
														   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
														   mInstances.size() * sizeof(InstanceData),
														   mInstances.data());
		}
	}

	uint32_t InstanceGroup::GetAssetId()
	{
		return mAssetId;
	}

	uint32_t InstanceGroup::GetNumInstances()
	{
		return mInstances.size();
	}

	Vk::Buffer* InstanceGroup::GetBuffer()
	{
		return mInstanceBuffer.get();
	}

	Vk::StaticModel* InstanceGroup::GetModel()
	{
		return mModel;
	}

	void RenderingManager::AddInstancedAsset(uint32_t assetId, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
	{
		// Instance group already exists?
		SharedPtr<InstanceGroup> instanceGroup = nullptr;
		for (uint32_t i = 0; i < mSceneInfo.instanceGroups.size(); i++)
		{
			if (mSceneInfo.instanceGroups[i]->GetAssetId() == assetId)
			{
				instanceGroup = mSceneInfo.instanceGroups[i];
				break;
			}
		}

		if (instanceGroup == nullptr)
		{
			// Todo: Check if assetId is valid
			instanceGroup = std::make_shared<InstanceGroup>(assetId);
			mSceneInfo.instanceGroups.push_back(instanceGroup);
		}

		instanceGroup->AddInstance(position, rotation, scale);
	}
	
	void RenderingManager::ClearInstanceGroups()
	{
		for (uint32_t i = 0; i < mSceneInfo.instanceGroups.size(); i++)
		{
			mSceneInfo.instanceGroups[i]->ClearInstances();
		}
	}

	void RenderingManager::BuildAllInstances()
	{
		for (uint32_t i = 0; i < mSceneInfo.instanceGroups.size(); i++)
		{
			mSceneInfo.instanceGroups[i]->BuildBuffer(mRenderer);
		}
	}

	void RenderingManager::RemoveRenderable(Renderable* renderable)
	{
		for (auto iter = mSceneInfo.renderables.begin(); iter != mSceneInfo.renderables.end(); iter++)
		{
			// Note: No need to free memory here since that will happen when the SharedPtr is removed from the CRenderable
			if ((*iter)->GetId() == renderable->GetId()) 
			{
				mSceneInfo.renderables.erase(iter);
				break;
			}
		}
	}
	void RenderingManager::RemoveLight(Light* light)
	{
		for (auto iter = mSceneInfo.lights.begin(); iter != mSceneInfo.lights.end(); iter++)
		{
			if ((*iter)->GetId() == light->GetId())
			{
				mSceneInfo.lights.erase(iter);
				break;
			}
		}
	}
	void RenderingManager::RemoveCamera(Camera* camera)
	{
		for (auto iter = mSceneInfo.cameras.begin(); iter != mSceneInfo.cameras.end(); iter++)
		{
			if ((*iter)->GetId() == camera->GetId())
			{
				mSceneInfo.cameras.erase(iter);
				break;
			}
		}
	}

	void RenderingManager::SetMainCamera(Camera* camera)
	{
		mRenderer->SetCamera(camera);
		mMainCamera = camera;
	}

	void RenderingManager::SetTerrain(Terrain* terrain)
	{
		mTerrain = terrain;
	}

	void RenderingManager::SetClippingPlane(glm::vec4 clippingPlane)
	{
		mClippingPlane = clippingPlane;
	}

	void RenderingManager::AddJob(BaseJob* job)
	{
		mJobs.push_back(job);
		job->Init(mJobs);
	}

	BaseTerrain* RenderingManager::GetTerrain()
	{
		return mSceneInfo.terrain.get();
	}

	RenderingSettings& RenderingManager::GetRenderingSettings()
	{
		return mRenderingSettings;
	}
}