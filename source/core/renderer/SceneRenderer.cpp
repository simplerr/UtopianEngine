#include <glm/gtc/matrix_transform.hpp>
#include "core/renderer/SceneRenderer.h"
#include "core/renderer/Renderable.h"
#include "vulkan/Renderer.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/DescriptorSet.h"
#include "Camera.h"
#include "vulkan/Mesh.h"
#include "vulkan/StaticModel.h"
#include "core/renderer/Light.h"
#include "core/terrain/Terrain.h"
#include "WaterRenderer.h"
#include "vulkan/ScreenGui.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/UIOverlay.h"

namespace Utopian
{
	SceneRenderer::SceneRenderer(Vk::Renderer* renderer)
	{
		mMainCamera = nullptr;
		mMainCamera = renderer->GetCamera();
		mRenderer = renderer;
		mCommandBuffer = mRenderer->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

		mWaterRenderer = new WaterRenderer(mRenderer, renderer->mModelLoader, renderer->mTextureLoader);
		mWaterRenderer->AddWater(glm::vec3(123000.0f, 0.0f, 106000.0f), 20);
		mWaterRenderer->AddWater(glm::vec3(103000.0f, 0.0f, 96000.0f), 20);

		mRenderer->AddScreenQuad(mRenderer->GetWindowWidth() - 2*350 - 50, mRenderer->GetWindowHeight() - 350, 300, 300, mWaterRenderer->GetReflectionRenderTarget()->GetImage(), mWaterRenderer->GetReflectionRenderTarget()->GetSampler());
		mRenderer->AddScreenQuad(mRenderer->GetWindowWidth() - 350, mRenderer->GetWindowHeight() - 350, 300, 300, mWaterRenderer->GetRefractionRenderTarget()->GetImage(), mWaterRenderer->GetRefractionRenderTarget()->GetSampler());

		mCubeModel = mRenderer->mModelLoader->LoadDebugBox(mRenderer->GetDevice());
	}

	SceneRenderer::~SceneRenderer()
	{
		delete mWaterRenderer;
	}

	void SceneRenderer::InitShaderResources()
	{
		for (auto& light : mLights)
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
		mCommonDescriptorSet->BindUniformBuffer(0, &per_frame_vs.GetDescriptor());
		mCommonDescriptorSet->BindUniformBuffer(1, &per_frame_ps.GetDescriptor());
		mCommonDescriptorSet->BindUniformBuffer(2, &fog_ubo.GetDescriptor());
		mCommonDescriptorSet->UpdateDescriptorSets();
	}

	void SceneRenderer::InitShader()
	{
		mPhongEffect.Init(mRenderer);
		mColorEffect.Init(mRenderer);
	}

	void SceneRenderer::Update()
	{
		per_frame_ps.lights.clear();
		for (auto& light : mLights)
		{
			per_frame_ps.lights.push_back(light->GetLightData());
		}

		mCommonDescriptorSet->BindUniformBuffer(0, &per_frame_vs.GetDescriptor());
		mCommonDescriptorSet->BindUniformBuffer(1, &per_frame_ps.GetDescriptor());
		mCommonDescriptorSet->BindUniformBuffer(2, &fog_ubo.GetDescriptor());
		mCommonDescriptorSet->UpdateDescriptorSets();
		mTerrain->Update();
		mWaterRenderer->Update(mRenderer, mMainCamera);
	}

	void SceneRenderer::RenderNodes(Vk::CommandBuffer* commandBuffer)
	{
		for (auto& renderable : mRenderables)
		{
			Vk::StaticModel* model = renderable->GetModel();
			mPhongEffect.SetPipeline(0);

			for (Vk::Mesh* mesh : model->mMeshes)
			{
				// Push the world matrix constant
				Vk::PushConstantBlock pushConsts(renderable->GetTransform().GetWorldMatrix());

				commandBuffer->CmdBindPipeline(mPhongEffect.GetPipeline());

				VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptor();
				VkDescriptorSet descriptorSets[2] = { mCommonDescriptorSet->descriptorSet, textureDescriptorSet };
				vkCmdBindDescriptorSets(commandBuffer->GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mPhongEffect.GetPipelineLayout(), 0, 2, descriptorSets, 0, NULL);

				commandBuffer->CmdPushConstants(&mPhongEffect, VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConsts), &pushConsts);

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
				//glm::vec3 translation = vec3(pos.x, pos.y, pos.z);
				//glm::vec3 translation = vec3(pos.x, pos.y - boundingBox.GetHeight() / 2, pos.z);
				glm::vec3 translation = vec3(pos.x, boundingBox.GetMin().y + boundingBox.GetHeight()/2, pos.z);
				mat4 world = glm::translate(glm::mat4(), translation);
				//world = glm::rotate(world, glm::radians(rotation.x), vec3(1.0f, 0.0f, 0.0f));
				//world = glm::rotate(world, glm::radians(rotation.y), vec3(0.0f, 1.0f, 0.0f));
				//world = glm::rotate(world, glm::radians(rotation.z), vec3(0.0f, 0.0f, 1.0f));
				world = glm::scale(world, glm::vec3(boundingBox.GetWidth(), boundingBox.GetHeight(), boundingBox.GetDepth()));

				Vk::PushConstantBlock pushConsts(world);

				commandBuffer->CmdBindPipeline(mColorEffect.GetPipeline());
				commandBuffer->CmdBindDescriptorSet(&mColorEffect, 1, &mColorEffect.mDescriptorSet0->descriptorSet, VK_PIPELINE_BIND_POINT_GRAPHICS);
				commandBuffer->CmdPushConstants(&mColorEffect, VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConsts), &pushConsts);
				commandBuffer->CmdBindVertexBuffer(0, 1, &mCubeModel->mMeshes[0]->vertices.buffer);
				commandBuffer->CmdBindIndexBuffer(mCubeModel->mMeshes[0]->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
				commandBuffer->CmdDrawIndexed(mCubeModel->GetNumIndices(), 1, 0, 0, 0);

				//ImGui::InputFloat3("Min", &boundingBox.GetMin(), 2);
			}
		}
	}

	void SceneRenderer::RenderScene(Vk::CommandBuffer* commandBuffer)
	{
		UpdateUniformBuffers();

		mTerrain->Render(commandBuffer, mCommonDescriptorSet);
		RenderNodes(commandBuffer);
	}

	void SceneRenderer::Render()
	{
		RenderOffscreen();

		mCommandBuffer->Begin(mRenderer->GetRenderPass(), mRenderer->GetCurrentFrameBuffer());
		mCommandBuffer->CmdSetViewPort(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());
		mCommandBuffer->CmdSetScissor(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());

		RenderScene(mCommandBuffer);

		mWaterRenderer->Render(mRenderer, mCommandBuffer);

		mCommandBuffer->End();
	}

	void SceneRenderer::RenderOffscreen()
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

	void SceneRenderer::UpdateUniformBuffers()
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
		mColorEffect.UpdateMemory(mRenderer->GetDevice());
	}

	void SceneRenderer::AddRenderable(Renderable* renderable)
	{
		mRenderables.push_back(renderable);
	}

	void SceneRenderer::AddLight(Light* light)
	{
		mLights.push_back(light);
	}

	void SceneRenderer::AddCamera(Camera* camera)
	{
		mCameras.push_back(camera);
	}

	void SceneRenderer::SetMainCamera(Camera* camera)
	{
		mRenderer->SetCamera(camera);
		mMainCamera = camera;
	}

	void SceneRenderer::SetTerrain(Terrain* terrain)
	{
		mTerrain = terrain;
	}

	void SceneRenderer::SetClippingPlane(glm::vec4 clippingPlane)
	{
		mClippingPlane = clippingPlane;
	}
}