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
#include "vulkan/ScreenGui.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/BasicRenderTarget.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/UIOverlay.h"

namespace Utopian
{
	RenderingManager::RenderingManager(Vk::Renderer* renderer)
	{
		mMainCamera = nullptr;
		mMainCamera = renderer->GetCamera();
		mRenderer = renderer;
		mCommandBuffer = mRenderer->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

		// To solve problem with RenderDoc not working with a secondary command buffer that is empty.
		// The real solution is to cleanup and remove this.
		mCommandBuffer->ToggleActive();

		mWaterRenderer = new WaterRenderer(mRenderer, renderer->mModelLoader, renderer->mTextureLoader);
		mWaterRenderer->AddWater(glm::vec3(123000.0f, 0.0f, 106000.0f), 20);
		mWaterRenderer->AddWater(glm::vec3(103000.0f, 0.0f, 96000.0f), 20);

		AddJob(new GBufferJob(renderer, renderer->GetWindowWidth(), renderer->GetWindowHeight()));
		AddJob(new SSAOJob(renderer, renderer->GetWindowWidth(), renderer->GetWindowHeight()));
		AddJob(new BlurJob(renderer, renderer->GetWindowWidth(), renderer->GetWindowHeight()));
		AddJob(new DeferredJob(renderer, renderer->GetWindowWidth(), renderer->GetWindowHeight()));

		// Default rendering settings
		mRenderingSettings.deferredPipeline = true;
		mRenderingSettings.fogColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
		mRenderingSettings.fogStart = 40000.0f;
		mRenderingSettings.fogDistance = 16000.0f;

		//mRenderer->AddScreenQuad(mRenderer->GetWindowWidth() - 2*350 - 50, mRenderer->GetWindowHeight() - 350, 300, 300, mWaterRenderer->GetReflectionImage(), mWaterRenderer->GetReflectionRenderTarget()->GetSampler());
		//mRenderer->AddScreenQuad(mRenderer->GetWindowWidth() - 350, mRenderer->GetWindowHeight() - 350, 300, 300, mWaterRenderer->GetRefractionImage(), mWaterRenderer->GetRefractionRenderTarget()->GetSampler());

		mCubeModel = mRenderer->mModelLoader->LoadDebugBox(mRenderer->GetDevice());
	}

	RenderingManager::~RenderingManager()
	{
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

		mEffects[Vk::EffectType::COLOR] = new Vk::ColorEffect();
		mEffects[Vk::EffectType::COLOR]->Init(mRenderer);
	}

	void RenderingManager::InitShader()
	{
		mPhongEffect.Init(mRenderer);
		mColorEffect.Init(mRenderer);
	}

	void RenderingManager::Update()
	{
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

		// Draw UI overlay for rendering settings
		// It's expected that each rendering node might have it's own settings that can be configured 
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::SetNextWindowPos(ImVec2(10, 150));
		ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Rendering settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::PushItemWidth(300.0f);

		ImGui::Checkbox("Deferred pipeline", &mRenderingSettings.deferredPipeline);
		ImGui::ColorEdit4("Fog color", &mRenderingSettings.fogColor.x);
		ImGui::SliderFloat("Fog start", &mRenderingSettings.fogStart, 0.0f, 100000.0f);
		ImGui::SliderFloat("Fog distance", &mRenderingSettings.fogDistance, 0.0f, 100000.0f);
		ImGui::SliderFloat("SSAO radius", &mRenderingSettings.ssaoRadius, 0.0f, 20.0f);
		ImGui::SliderFloat("SSAO bias", &mRenderingSettings.ssaoBias, 0.0f, 10.0f);
		ImGui::SliderInt("SSAO blur radius", &mRenderingSettings.blurRadius, 1, 20);

		ImGui::PopItemWidth();

		// ImGui functions end here
		ImGui::End();
		ImGui::PopStyleVar();
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

				VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptor();
				VkDescriptorSet descriptorSets[2] = { mCommonDescriptorSet->descriptorSet, textureDescriptorSet };
				vkCmdBindDescriptorSets(commandBuffer->GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mEffects[Vk::EffectType::PHONG]->GetPipelineLayout(), 0, 2, descriptorSets, 0, NULL);

				commandBuffer->CmdPushConstants(mEffects[Vk::EffectType::PHONG], VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConsts), &pushConsts);

				commandBuffer->CmdBindVertexBuffer(0, 1, &mesh->vertices.buffer);
				commandBuffer->CmdBindIndexBuffer(mesh->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
				commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
			}

			// Draw the AABB
			if (renderable->IsBoundingBoxVisible())
			{
				BoundingBox boundingBox = renderable->GetBoundingBox();
				vec3 pos = renderable->GetTransform().GetPosition();
				vec3 rotation = renderable->GetTransform().GetRotation();
				glm::vec3 translation = vec3(pos.x, boundingBox.GetMin().y + boundingBox.GetHeight()/2, pos.z);
				mat4 world = glm::translate(glm::mat4(), translation);
				world = glm::scale(world, glm::vec3(boundingBox.GetWidth(), boundingBox.GetHeight(), boundingBox.GetDepth()));

				Vk::PushConstantBlock pushConsts(world, vec4(1, 0, 0, 1));

				commandBuffer->CmdBindPipeline(mColorEffect.GetPipeline(0));
				commandBuffer->CmdBindDescriptorSet(&mColorEffect, 1, &mColorEffect.mDescriptorSet0->descriptorSet, VK_PIPELINE_BIND_POINT_GRAPHICS);
				commandBuffer->CmdPushConstants(&mColorEffect, VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConsts), &pushConsts);
				commandBuffer->CmdBindVertexBuffer(0, 1, &mCubeModel->mMeshes[0]->vertices.buffer);
				commandBuffer->CmdBindIndexBuffer(mCubeModel->mMeshes[0]->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
				commandBuffer->CmdDrawIndexed(mCubeModel->GetNumIndices(), 1, 0, 0, 0);
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

			mColorEffect.per_frame_vs.data.projection = mMainCamera->GetProjection();
			mColorEffect.per_frame_vs.data.view = mMainCamera->GetView();
		}

		fog_ubo.data.fogColor = mRenderer->GetClearColor();
		fog_ubo.data.fogStart = 41500.0f;
		fog_ubo.data.fogDistance = 15400.0f;

		per_frame_vs.UpdateMemory();
		per_frame_ps.UpdateMemory();
		fog_ubo.UpdateMemory();
		mColorEffect.UpdateMemory();
	}

	void RenderingManager::AddRenderable(Renderable* renderable)
	{
		mSceneInfo.renderables.push_back(renderable);
	}

	void RenderingManager::AddLight(Light* light)
	{
		mSceneInfo.lights.push_back(light);
	}

	void RenderingManager::AddCamera(Camera* camera)
	{
		mSceneInfo.cameras.push_back(camera);
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
}