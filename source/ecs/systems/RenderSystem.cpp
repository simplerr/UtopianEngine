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
#include "Camera.h"
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

		mCubeModel = mModelLoader->LoadQuad(renderer->GetDevice());

		AddDebugCube(vec3(77000.0f, 8500.0f, 67000.0f), Vulkan::Color::Red, 12000.0f);
		AddDebugCube(vec3(2000.0f, 0.0f, 0.0f), Vulkan::Color::Red, 70.0f);
		AddDebugCube(vec3(0.0f, 2000.0f, 0.0f), Vulkan::Color::Green, 70.0f);
		AddDebugCube(vec3(0.0f, 0.0f, 2000.0f), Vulkan::Color::Blue, 70.0f);

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

		PrepareOffscreen();
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
	}

	void RenderSystem::PrepareOffscreen()
	{
		offscreen.width = 512;
		offscreen.height = 512;

		offscreen.commandBuffer = new Vulkan::CommandBuffer(mRenderer->GetDevice(), mRenderer->GetCommandPool(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, false);
		offscreen.colorImage = new Vulkan::ImageColor(mRenderer->GetDevice(), offscreen.width, offscreen.height, VK_FORMAT_R8G8B8A8_UNORM);
		offscreen.depthImage = new Vulkan::ImageDepth(mRenderer->GetDevice(), offscreen.width, offscreen.height, VK_FORMAT_D32_SFLOAT_S8_UINT);
		offscreen.renderPass = new Vulkan::RenderPass(mRenderer->GetDevice(), VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		offscreen.frameBuffer = new Vulkan::FrameBuffers(mRenderer->GetDevice(), offscreen.renderPass, offscreen.depthImage, offscreen.colorImage, offscreen.width, offscreen.height);
		offscreen.sampler = new Vulkan::Sampler(mRenderer->GetDevice());

		offscreen.textureDescriptorSet = new Vulkan::DescriptorSet(mRenderer->GetDevice(), mRenderer->GetTextureDescriptorSetLayout(), mRenderer->GetDescriptorPool());
		offscreen.textureDescriptorSet->BindCombinedImage(0, offscreen.colorImage, offscreen.sampler);	// NOTE: It's hard to know that the texture must be bound to binding=0
		offscreen.textureDescriptorSet->UpdateDescriptorSets();
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
			
			for (Vulkan::Mesh* mesh : model->mMeshes)
			{
				mCommandBuffer->CmdBindPipeline(mPhongEffect.GetPipeline());

				VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptor();
				textureDescriptorSet = offscreen.textureDescriptorSet->descriptorSet;

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
				//mCommandBuffer->CmdBindPipeline(mGeometryPipeline);
				//vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout->GetVkHandle(), 0, 1, &mDescriptorSet->descriptorSet, 0, NULL);
				//mCommandBuffer->CmdPushConstants(mPipelineLayout, VK_SHADER_STAGE_GEOMETRY_BIT, sizeof(pushConstantBlock), &pushConstantBlock);
				//mCommandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
			}
		}

		// Draw debug cubes for the origin and each axis
		for (int i = 0; i < mDebugCubes.size(); i++)
		{
			DebugCube debugCube = mDebugCubes[i];

			glm::mat4 world = mat4();
			world = glm::translate(world, debugCube.pos);
			world = glm::scale(world, glm::vec3(debugCube.size));

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

		mCommandBuffer->End();
	}

	void RenderSystem::RenderOffscreen()
	{
		VkClearValue clearValues[2];
		clearValues[0].color = { 0.2f, 0.2f, 0.2f, 0.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = offscreen.renderPass->GetVkHandle();
		renderPassBeginInfo.renderArea.extent.width = offscreen.width;
		renderPassBeginInfo.renderArea.extent.height = offscreen.height;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = offscreen.frameBuffer->GetFrameBuffer(0); // TODO: NOTE: Should not be like this

		// Begin command buffer recording & the render pass
		offscreen.commandBuffer->Begin();
		offscreen.commandBuffer->CmdBeginRenderPass(renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		offscreen.commandBuffer->CmdSetViewPort(offscreen.width, offscreen.height);
		offscreen.commandBuffer->CmdSetScissor(offscreen.width, offscreen.height);

		mTerrain->Render(offscreen.commandBuffer);

		offscreen.commandBuffer->CmdEndRenderPass();

		offscreen.commandBuffer->Flush(mRenderer->GetQueue()->GetVkHandle(), mRenderer->GetCommandPool());
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