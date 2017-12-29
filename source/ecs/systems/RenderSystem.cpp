#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "ecs/Entity.h"
#include "ecs/components/MeshComponent.h"
#include "ecs/components/TransformComponent.h"
#include "vulkan/Renderer.h"
#include "vulkan/ShaderManager.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/Pipeline.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/Queue.h"
#include "vulkan/handles/RenderPass.h"
#include "vulkan/handles/FrameBuffers.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Sampler.h"
#include "vulkan/Mesh.h"
#include "vulkan/ScreenGui.h"
#include "vulkan/RenderTarget.h"
#include "Camera.h"
#include "WaterRenderer.h"
#include "vulkan/StaticModel.h"
#include "vulkan/VertexDescription.h"
#include "RenderSystem.h"
#include "Colors.h"
#include "Terrain.h"
#include "Light.h"

#define VERTEX_BUFFER_BIND_ID 0

namespace ECS
{
	RenderSystem::RenderSystem(SystemManager* entityManager, Vulkan::Renderer* renderer, Vulkan::Camera* camera)
		: System(entityManager, Type::MESH_COMPONENT | Type::TRANSFORM_COMPONENT)
	{
		mRenderer = renderer;
		mCamera = camera;
		mTextureLoader = mRenderer->mTextureLoader;
		mModelLoader = new Vulkan::ModelLoader(mTextureLoader);

		mCubeModel = mModelLoader->LoadDebugBox(renderer->GetDevice());

		//AddDebugCube(vec3(92000.0f, 0.0f, 80000.0f), Vulkan::Color::Red, 1.0f);
		//AddDebugCube(vec3(2000.0f, 0.0f, 0.0f), Vulkan::Color::Red, 70.0f);
		//AddDebugCube(vec3(0.0f, 2000.0f, 0.0f), Vulkan::Color::Green, 70.0f);
		//AddDebugCube(vec3(0.0f, 0.0f, 2000.0f), Vulkan::Color::Blue, 70.0f);

		mCommandBuffer = mRenderer->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);
		mTerrainCommandBuffer = mRenderer->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

		//
		// Terrain
		// 
		mTerrain = new Terrain(mRenderer, mCamera);
		//mCommandBuffer->ToggleActive();

		// NOTE: This must run before PhongEffect::Init()
		// Create the fragment shader uniform buffer
		Vulkan::Light light;
		light.SetMaterials(vec4(1, 1, 1, 1), vec4(1, 1, 1, 1), vec4(1, 1, 1, 32));
		light.SetPosition(600, -800, 600);
		light.SetDirection(1, -1, 1);
		light.SetAtt(1, 0, 0);
		light.SetIntensity(0.2f, 0.8f, 1.0f);
		light.SetType(Vulkan::LightType::DIRECTIONAL_LIGHT);
		light.SetRange(100000);
		light.SetSpot(4.0f);
		mPhongEffect.per_frame_ps.lights.push_back(light);

		light.SetMaterials(vec4(1, 0, 0, 1), vec4(1, 0, 0, 1), vec4(1, 0, 0, 32));
		light.SetPosition(600, -800, 600);
		light.SetDirection(-1, -1, -1);
		light.SetAtt(1, 0, 0);
		light.SetIntensity(0.2f, 0.5f, 1.0f);
		light.SetType(Vulkan::LightType::SPOT_LIGHT);
		light.SetRange(100000);
		light.SetSpot(4.0f);
		mPhongEffect.per_frame_ps.lights.push_back(light);

		// Important to call this before Create() since # lights affects the total size
		mPhongEffect.per_frame_ps.constants.numLights = mPhongEffect.per_frame_ps.lights.size();

		mPhongEffect.Init(mRenderer);

		//
		// Geometry shader pipeline
		//
		mDescriptorSetLayout = new Vulkan::DescriptorSetLayout(mRenderer->GetDevice());
		mDescriptorSetLayout->AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_GEOMETRY_BIT);
		mDescriptorSetLayout->Create();

		mPipelineLayout = new Vulkan::PipelineLayout(mRenderer->GetDevice());
		mPipelineLayout->AddDescriptorSetLayout(mDescriptorSetLayout);
		mPipelineLayout->AddPushConstantRange(VK_SHADER_STAGE_GEOMETRY_BIT, sizeof(PushConstantBlock));
		mPipelineLayout->Create();

		mDescriptorPool = new Vulkan::DescriptorPool(mRenderer->GetDevice());
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
		mDescriptorPool->Create();

		mUniformBuffer.Create(mRenderer->GetDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		mDescriptorSet = new Vulkan::DescriptorSet(mRenderer->GetDevice(), mDescriptorSetLayout, mDescriptorPool);
		mDescriptorSet->BindUniformBuffer(0, &mUniformBuffer.GetDescriptor());
		mDescriptorSet->UpdateDescriptorSets();

		Vulkan::Shader* shader = mRenderer->mShaderManager->CreateShader("data/shaders/geometry/base.vert.spv", "data/shaders/geometry/base.frag.spv", "data/shaders/geometry/normaldebug.geom.spv");
		mGeometryPipeline = new Vulkan::Pipeline(mRenderer->GetDevice(), mPipelineLayout, mRenderer->GetRenderPass(), mPhongEffect.GetVertexDescription(), shader);
		mGeometryPipeline->Create();

		mWaterRenderer = new WaterRenderer(mRenderer, mModelLoader, mTextureLoader);
		mWaterRenderer->AddWater(glm::vec3(123000.0f, 0.0f, 106000.0f), 20);
		mWaterRenderer->AddWater(glm::vec3(103000.0f, 0.0f, 96000.0f), 20);

		mScreenGui = new Vulkan::ScreenGui(mRenderer);
		mScreenGui->AddQuad(mRenderer->GetWindowWidth() - 2*350 - 50, mRenderer->GetWindowHeight() - 350, 300, 300, mWaterRenderer->GetReflectionRenderTarget()->GetImage(), mWaterRenderer->GetReflectionRenderTarget()->GetSampler());
		mScreenGui->AddQuad(mRenderer->GetWindowWidth() - 350, mRenderer->GetWindowHeight() - 350, 300, 300, mWaterRenderer->GetRefractionRenderTarget()->GetImage(), mWaterRenderer->GetRefractionRenderTarget()->GetSampler());
	}

	RenderSystem::~RenderSystem()
	{
		mModelLoader->CleanupModels(mRenderer->GetVkDevice());
		delete mModelLoader;
		delete mDescriptorSetLayout;
		delete mPipelineLayout;
		delete mDescriptorPool;
		delete mDescriptorSet;
		delete mGeometryPipeline;
		delete mTerrain;
		delete mWaterRenderer;
	}

	void RenderSystem::OnEntityAdded(const EntityCache& entityCache)
	{
		// Load the model
		Vulkan::StaticModel* model = mModelLoader->LoadModel(mRenderer->GetDevice(), entityCache.meshComponent->GetFilename());

		entityCache.meshComponent->SetModel(model);
	}

	void RenderSystem::Process()
	{
		mTerrain->Update();

		mTerrainCommandBuffer->Begin(mRenderer->GetRenderPass(), mRenderer->GetCurrentFrameBuffer());
		mTerrainCommandBuffer->CmdSetViewPort(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());
		mTerrainCommandBuffer->CmdSetScissor(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());

		mTerrain->Render(mTerrainCommandBuffer);
		mTerrainCommandBuffer->End();

		RenderOffscreen();
		
		// From Renderer.cpp
		if (mCamera != nullptr)
		{
			mPhongEffect.per_frame_vs.camera.projectionMatrix = mCamera->GetProjection();
			mPhongEffect.per_frame_vs.camera.viewMatrix = mCamera->GetView();
			mPhongEffect.per_frame_vs.camera.projectionMatrix = mCamera->GetProjection();
			mPhongEffect.per_frame_vs.camera.eyePos = mCamera->GetPosition();

			mWaterRenderer->Update(mRenderer, mCamera);
		}

		mPhongEffect.UpdateMemory(mRenderer->GetDevice());

		// TEMP:
		mUniformBuffer.data.projection = mCamera->GetProjection();
		mUniformBuffer.data.view = mCamera->GetView();
		mUniformBuffer.UpdateMemory(mRenderer->GetVkDevice());

		// Temp
		VkCommandBuffer commandBuffer = mCommandBuffer->GetVkHandle();

		// Build mesh rendering command buffer
		mCommandBuffer->Begin(mRenderer->GetRenderPass(), mRenderer->GetCurrentFrameBuffer());

		mCommandBuffer->CmdSetViewPort(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());
		mCommandBuffer->CmdSetScissor(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());

		for (EntityCache entityCache : mEntities)
		{
			Vulkan::StaticModel* model = entityCache.meshComponent->GetModel();
			mPhongEffect.SetPipeline(entityCache.meshComponent->GetPipeline());
			//mPhongEffect.SetPipeline(Vulkan::PipelineType::PIPELINE_WIREFRAME);
			
			for (Vulkan::Mesh* mesh : model->mMeshes)
			{
				mCommandBuffer->CmdBindPipeline(mPhongEffect.GetPipeline());

				VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptor();

				VkDescriptorSet descriptorSets[3] = { mPhongEffect.mCameraDescriptorSet->descriptorSet, mPhongEffect.mLightDescriptorSet->descriptorSet, textureDescriptorSet };
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPhongEffect.GetPipelineLayout(), 0, 3, descriptorSets, 0, NULL);

				// Push the world matrix constant
				Vulkan::PushConstantBlock pushConstantBlock;
				pushConstantBlock.world = entityCache.transformComponent->GetWorldMatrix();
				pushConstantBlock.worldInvTranspose = entityCache.transformComponent->GetWorldInverseTransposeMatrix(); // TOOD: This probably also needs to be negated

				// NOTE: For some reason the translation needs to be negated when rendering
				// Otherwise the physical representation does not match the rendered scene
				pushConstantBlock.world[3][0] = -pushConstantBlock.world[3][0];
				pushConstantBlock.world[3][1] = -pushConstantBlock.world[3][1];
				pushConstantBlock.world[3][2] = -pushConstantBlock.world[3][2];

				mCommandBuffer->CmdPushConstants(&mPhongEffect, VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConstantBlock), &pushConstantBlock);

				mCommandBuffer->CmdBindVertexBuffer(VERTEX_BUFFER_BIND_ID, 1, &mesh->vertices.buffer);
				mCommandBuffer->CmdBindIndexBuffer(mesh->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
				mCommandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);

				// Try the geometry shader pipeline
				/*mCommandBuffer->CmdBindPipeline(mGeometryPipeline);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout->GetVkHandle(), 0, 1, &mDescriptorSet->descriptorSet, 0, NULL);
				mCommandBuffer->CmdPushConstants(mPipelineLayout, VK_SHADER_STAGE_GEOMETRY_BIT, sizeof(pushConstantBlock), &pushConstantBlock);
				mCommandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);*/
			}
		}

		// Draw debug cubes for the origin and each axis
		for (int i = 0; i < mDebugCubes.size(); i++)
		{
			DebugCube debugCube = mDebugCubes[i];

			glm::mat4 world = mat4();
			world = glm::translate(world, debugCube.pos);
			world = glm::scale(world, glm::vec3(debugCube.size));
			/*world = glm::rotate(world, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
			world = glm::rotate(world, glm::radians(0.0f), vec3(0.0f, 1.0f, 0.0f));
			world = glm::rotate(world, glm::radians(0.0f), vec3(0.0f, 0.0f, 1.0f));*/

			PushConstantDebugBlock pushConstantBlock;
			pushConstantBlock.world = world;
			pushConstantBlock.color = debugCube.color;
			pushConstantBlock.world[3][0] = -pushConstantBlock.world[3][0];
			pushConstantBlock.world[3][1] = -pushConstantBlock.world[3][1];
			pushConstantBlock.world[3][2] = -pushConstantBlock.world[3][2];

			mCommandBuffer->CmdPushConstants(&mPhongEffect, VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConstantBlock), &pushConstantBlock);
			mCommandBuffer->CmdBindVertexBuffer(VERTEX_BUFFER_BIND_ID, 1, &mCubeModel->mMeshes[0]->vertices.buffer);
			mCommandBuffer->CmdBindIndexBuffer(mCubeModel->mMeshes[0]->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
			mCommandBuffer->CmdDrawIndexed(mCubeModel->GetNumIndices(), 1, 0, 0, 0);
		}

		mScreenGui->Render(mRenderer, mCommandBuffer);

		//
		// Render water
		//
		mWaterRenderer->Render(mRenderer, mCommandBuffer);

		mCommandBuffer->End();
	}

	void RenderSystem::RenderOffscreen()
	{
		glm::vec3 cameraPos = mCamera->GetPosition();

		// Reflection renderpass
		mCamera->SetPosition(mCamera->GetPosition() - glm::vec3(0, mCamera->GetPosition().y *  2, 0)); // NOTE: Water is hardcoded to be at y = 0
		mCamera->SetOrientation(mCamera->GetYaw(), -mCamera->GetPitch());
		mTerrain->SetClippingPlane(glm::vec4(0, -1, 0, 0));
		mTerrain->UpdateUniformBuffer();

		mWaterRenderer->GetReflectionRenderTarget()->Begin();	

		mTerrain->Render(mWaterRenderer->GetReflectionRenderTarget()->GetCommandBuffer());

		mWaterRenderer->GetReflectionRenderTarget()->End(mRenderer->GetQueue());

		// Refraction renderpass
		mTerrain->SetClippingPlane(glm::vec4(0, 1, 0, 0));
		mCamera->SetPosition(cameraPos);
		mCamera->SetOrientation(mCamera->GetYaw(), -mCamera->GetPitch());
		mTerrain->UpdateUniformBuffer();

		mWaterRenderer->GetRefractionRenderTarget()->Begin();	

		mTerrain->Render(mWaterRenderer->GetRefractionRenderTarget()->GetCommandBuffer());

		mWaterRenderer->GetRefractionRenderTarget()->End(mRenderer->GetQueue());

		mTerrain->SetClippingPlane(glm::vec4(0, 1, 0, 1500000));
		mTerrain->UpdateUniformBuffer();
	}


	void RenderSystem::AddDebugCube(vec3 pos, vec4 color, float size)
	{
		mDebugCubes.push_back(DebugCube(pos, color, size));
	}

	void RenderSystem::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		mTerrain->HandleMessages(hWnd, uMsg, wParam, lParam);
	}
}