#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/gtc/matrix_transform.hpp>
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
#include "vulkan/Mesh.h"
#include "Camera.h"
#include "vulkan/StaticModel.h"
#include "vulkan/VertexDescription.h"
#include "RenderSystem.h"
#include "Colors.h"
#include "Terrain.h"

#define VERTEX_BUFFER_BIND_ID 0

namespace ECS
{
	RenderSystem::RenderSystem(EntityManager* entityManager, Vulkan::Renderer* renderer, Vulkan::Camera* camera)
		: System(entityManager, Type::MESH_COMPONENT | Type::TRANSFORM_COMPONENT)
	{
		mRenderer = renderer;
		mCamera = camera;
		mTextureLoader = mRenderer->mTextureLoader;
		mModelLoader = new Vulkan::ModelLoader(mTextureLoader);

		mCubeModel = mModelLoader->LoadDebugBox(renderer->GetDevice());

		AddDebugCube(vec3(0.0f, 0.0f, 0.0f), Vulkan::Color::White, 70.0f);
		AddDebugCube(vec3(2000.0f, 0.0f, 0.0f), Vulkan::Color::Red, 70.0f);
		AddDebugCube(vec3(0.0f, 2000.0f, 0.0f), Vulkan::Color::Green, 70.0f);
		AddDebugCube(vec3(0.0f, 0.0f, 2000.0f), Vulkan::Color::Blue, 70.0f);

		mCommandBuffer = mRenderer->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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
		mDescriptorSet->AllocateDescriptorSets();
		mDescriptorSet->BindUniformBuffer(0, &mUniformBuffer.GetDescriptor());
		mDescriptorSet->UpdateDescriptorSets();

		Vulkan::Shader* shader = mRenderer->mShaderManager->CreateShader("data/shaders/geometry/base.vert.spv", "data/shaders/geometry/base.frag.spv", "data/shaders/geometry/normaldebug.geom.spv");
		mGeometryPipeline = new Vulkan::Pipeline(mRenderer->GetDevice(), mPipelineLayout, mRenderer->GetRenderPass(), mRenderer->GetVertexDescription(), shader);
		mGeometryPipeline->Create();

		//
		// Terrain
		// 
		mTerrain = new Terrain(mRenderer, mCamera);
		mCommandBuffer->ToggleActive();
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

	void RenderSystem::OnEntityAdded(const EntityCache& entityCache)
	{
		// Load the model
		Vulkan::StaticModel* model = mModelLoader->LoadModel(mRenderer->GetDevice(), entityCache.meshComponent->GetFilename());

		entityCache.meshComponent->SetModel(model);
	}

	void RenderSystem::Process()
	{
		mTerrain->Update();

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
			
			for (Vulkan::Mesh* mesh : model->mMeshes)
			{
				mCommandBuffer->CmdBindPipeline(mRenderer->GetPipeline(entityCache.meshComponent->GetPipeline()));

				VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptor();

				VkDescriptorSet descriptorSets[3] = { mRenderer->mCameraDescriptorSet->descriptorSet, mRenderer->mLightDescriptorSet->descriptorSet, textureDescriptorSet };
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderer->GetPipelineLayout()->GetVkHandle(), 0, 3, descriptorSets, 0, NULL);

				// Push the world matrix constant
				Vulkan::PushConstantBlock pushConstantBlock;
				pushConstantBlock.world = entityCache.transformComponent->GetWorldMatrix();
				pushConstantBlock.worldInvTranspose = entityCache.transformComponent->GetWorldInverseTransposeMatrix(); // TOOD: This probably also needs to be negated

				// NOTE: For some reason the translation needs to be negated when rendering
				// Otherwise the physical representation does not match the rendered scene
				pushConstantBlock.world[3][0] = -pushConstantBlock.world[3][0];
				pushConstantBlock.world[3][1] = -pushConstantBlock.world[3][1];
				pushConstantBlock.world[3][2] = -pushConstantBlock.world[3][2];

				mCommandBuffer->CmdPushConstants(mRenderer->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConstantBlock), &pushConstantBlock);

				mCommandBuffer->CmdBindVertexBuffer(VERTEX_BUFFER_BIND_ID, 1, &mesh->vertices.buffer);
				mCommandBuffer->CmdBindIndexBuffer(mesh->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
				mCommandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);

				// Try the geometry shader pipeline
				mCommandBuffer->CmdBindPipeline(mGeometryPipeline);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout->GetVkHandle(), 0, 1, &mDescriptorSet->descriptorSet, 0, NULL);
				mCommandBuffer->CmdPushConstants(mPipelineLayout, VK_SHADER_STAGE_GEOMETRY_BIT, sizeof(pushConstantBlock), &pushConstantBlock);
				mCommandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
			}
		}

		// Draw debug boxes
		//for (EntityCache entityCache : mEntities)
		//{
		//	mCommandBuffer->CmdBindPipeline(mRenderer->GetPipeline(VulkanLib::PipelineType::PIPELINE_TEST));
		//	//mCommandBuffer->CmdBindDescriptorSet(mRenderer->GetPipelineLayout(), mRenderer->GetDescriptorSet());

		//	//VkDescriptorSet descriptorSets[3] = { mRenderer->mCameraDescriptorSet->descriptorSet, mRenderer->mLightDescriptorSet->descriptorSet, mRenderer->mTextureDescriptorSet->descriptorSet };
		//	//vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderer->GetPipelineLayout()->GetVkHandle(), 0, 3, descriptorSets, 0, NULL);

		//	VulkanLib::BoundingBox meshBoundingBox = entityCache.meshComponent->GetBoundingBox();
		//	meshBoundingBox.Update(entityCache.transformComponent->GetWorldMatrix()); 

		//	mat4 world = mat4();
		//	float width = meshBoundingBox.GetWidth();
		//	float height = meshBoundingBox.GetHeight();
		//	float depth = meshBoundingBox.GetDepth();
		//	world = glm::translate(world, -entityCache.transformComponent->GetPosition());
		//	world = glm::scale(world, glm::vec3(width, height, depth));

		//	PushConstantDebugBlock pushConstantBlock;
		//	pushConstantBlock.world = world;
		//	pushConstantBlock.color = VulkanLib::Color::White;

		//	mCommandBuffer->CmdPushConstants(mRenderer->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConstantBlock), &pushConstantBlock);
		//	mCommandBuffer->CmdBindVertexBuffer(VERTEX_BUFFER_BIND_ID, 1, &mCubeModel->mMeshes[0]->vertices.buffer);
		//	mCommandBuffer->CmdBindIndexBuffer(mCubeModel->mMeshes[0]->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
		//	mCommandBuffer->CmdDrawIndexed(mCubeModel->GetNumIndices(), 1, 0, 0, 0);
		//}

		//mCommandBuffer->CmdBindPipeline(mRenderer->GetPipeline(VulkanLib::PipelineType::PIPELINE_DEBUG));
		////mCommandBuffer->CmdBindDescriptorSet(mRenderer->GetPipelineLayout(), mRenderer->GetDescriptorSet());

		////VkDescriptorSet descriptorSets[3] = { mRenderer->mCameraDescriptorSet->descriptorSet, mRenderer->mLightDescriptorSet->descriptorSet, mRenderer->mTestTexture.descriptorSet->descriptorSet };
		////vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderer->GetPipelineLayout()->GetVkHandle(), 0, 3, descriptorSets, 0, NULL);

		//// Draw debug cubes for the origin and each axis
		//for (int i = 0; i < mDebugCubes.size(); i++)
		//{
		//	DebugCube debugCube = mDebugCubes[i];

		//	glm::mat4 world = mat4();
		//	world = glm::translate(world, -debugCube.pos);
		//	world = glm::scale(world, glm::vec3(debugCube.size));

		//	PushConstantDebugBlock pushConstantBlock;
		//	pushConstantBlock.world = world;
		//	pushConstantBlock.color = debugCube.color;

		//	mCommandBuffer->CmdPushConstants(mRenderer->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConstantBlock), &pushConstantBlock);
		//	mCommandBuffer->CmdBindVertexBuffer(VERTEX_BUFFER_BIND_ID, 1, &mCubeModel->mMeshes[0]->vertices.buffer);
		//	mCommandBuffer->CmdBindIndexBuffer(mCubeModel->mMeshes[0]->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
		//	mCommandBuffer->CmdDrawIndexed(mCubeModel->GetNumIndices(), 1, 0, 0, 0);
		//}

		mCommandBuffer->End();
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