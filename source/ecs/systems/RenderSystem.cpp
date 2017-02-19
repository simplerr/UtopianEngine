#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/gtc/matrix_transform.hpp>
#include "ecs/Entity.h"
#include "ecs/components/MeshComponent.h"
#include "ecs/components/TransformComponent.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/Pipeline.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/Mesh.h"
#include "RenderSystem.h"
#include "Colors.h"

#define VERTEX_BUFFER_BIND_ID 0

namespace ECS
{
	RenderSystem::RenderSystem(EntityManager* entityManager, VulkanLib::VulkanApp* vulkanApp)
		: System(entityManager, Type::MESH_COMPONENT | Type::TRANSFORM_COMPONENT)
	{
		mVulkanApp = vulkanApp;
		mModelLoader = new VulkanLib::ModelLoader();

		mCubeModel = mModelLoader->LoadDebugBox(vulkanApp->GetDeviceTmp());

		AddDebugCube(vec3(0.0f, 0.0f, 0.0f), VulkanLib::Color::White, 70.0f);
		AddDebugCube(vec3(2000.0f, 0.0f, 0.0f), VulkanLib::Color::Red, 70.0f);
		AddDebugCube(vec3(0.0f, 2000.0f, 0.0f), VulkanLib::Color::Green, 70.0f);
		AddDebugCube(vec3(0.0f, 0.0f, 2000.0f), VulkanLib::Color::Blue, 70.0f);
	}

	RenderSystem::~RenderSystem()
	{
		mModelLoader->CleanupModels(mVulkanApp->GetDevice());
		delete mModelLoader;
	}

	void RenderSystem::Render(VulkanLib::CommandBuffer* commandBuffer, std::map<int, VulkanLib::Pipeline*>& pipelines, VulkanLib::PipelineLayout* pipelineLayout, VulkanLib::DescriptorSet& descriptorSet)
	{
		// Draw all meshes
		for (EntityCache entityCache : mEntities)
		{
			// Bind the rendering pipeline (including the shaders)
			vkCmdBindPipeline(commandBuffer->GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[entityCache.meshComponent->GetPipeline()]->GetVkHandle());

			// Bind descriptor sets describing shader binding points (must be called after vkCmdBindPipeline!)
			vkCmdBindDescriptorSets(commandBuffer->GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->GetVkHandle(), 0, 1, &descriptorSet.descriptorSet, 0, NULL);

			TransformComponent* transform = entityCache.transformComponent;
			MeshComponent* meshComponent = entityCache.meshComponent;
			VulkanLib::StaticModel* model = entityCache.meshComponent->GetModel();

			// Bind the correct vertex & index buffers
			// Push the world matrix constant
			VulkanLib::PushConstantBlock pushConstantBlock;
			pushConstantBlock.world = transform->GetWorldMatrix();

			// NOTE: For some reason the translation needs to be negated when rendering
			// Otherwise the physical representation does not match the rendered scene
			pushConstantBlock.world[3][0] = -pushConstantBlock.world[3][0];	
			pushConstantBlock.world[3][1] = -pushConstantBlock.world[3][1];
			pushConstantBlock.world[3][2] = -pushConstantBlock.world[3][2];
				
			// TOOD: This probably also needs to be negated
			pushConstantBlock.worldInvTranspose = transform->GetWorldInverseTransposeMatrix();
			vkCmdPushConstants(commandBuffer->GetVkHandle(), pipelineLayout->GetVkHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushConstantBlock), &pushConstantBlock);

			// Bind triangle vertices
			VkDeviceSize offsets[1] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer->GetVkHandle(), VERTEX_BUFFER_BIND_ID, 1, &model->mMeshes[0]->vertices.buffer, offsets);		
			vkCmdBindIndexBuffer(commandBuffer->GetVkHandle(), model->mMeshes[0]->indices.buffer, 0, VK_INDEX_TYPE_UINT32);

			// Draw indexed triangle	
			vkCmdDrawIndexed(commandBuffer->GetVkHandle(), model->GetNumIndices(), 1, 0, 0, 0);
		}

		// Draw debug boxes
		for (EntityCache entityCache : mEntities)
		{
			// Bind the rendering pipeline (including the shaders)
			vkCmdBindPipeline(commandBuffer->GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[VulkanLib::PipelineType::PIPELINE_TEST]->GetVkHandle());

			// Bind descriptor sets describing shader binding points (must be called after vkCmdBindPipeline!)
			vkCmdBindDescriptorSets(commandBuffer->GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->GetVkHandle(), 0, 1, &descriptorSet.descriptorSet, 0, NULL);

			// Draw debug bounding box
			VulkanLib::BoundingBox meshBoundingBox = entityCache.meshComponent->GetBoundingBox();
			meshBoundingBox.Update(entityCache.transformComponent->GetWorldMatrix()); 

			mat4 world = mat4();
			float width = meshBoundingBox.GetWidth();
			float height = meshBoundingBox.GetHeight();
			float depth = meshBoundingBox.GetDepth();
			world = glm::translate(world, -entityCache.transformComponent->GetPosition());
			world = glm::scale(world, glm::vec3(width, height, depth));

			PushConstantDebugBlock pushConstantBlock;
			pushConstantBlock.world = world;
			pushConstantBlock.color = VulkanLib::Color::White;

			VkDeviceSize offsets[1] = { 0 };
			vkCmdPushConstants(commandBuffer->GetVkHandle(), pipelineLayout->GetVkHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushConstantBlock), &pushConstantBlock);
			vkCmdBindVertexBuffers(commandBuffer->GetVkHandle(), VERTEX_BUFFER_BIND_ID, 1, &mCubeModel->mMeshes[0]->vertices.buffer, offsets);
			vkCmdBindIndexBuffer(commandBuffer->GetVkHandle(), mCubeModel->mMeshes[0]->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer->GetVkHandle(), mCubeModel->GetNumIndices(), 1, 0, 0, 0);
		}

		// Bind the rendering pipeline (including the shaders)
		vkCmdBindPipeline(commandBuffer->GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[VulkanLib::PipelineType::PIPELINE_DEBUG]->GetVkHandle());

		// Bind descriptor sets describing shader binding points (must be called after vkCmdBindPipeline!)
		vkCmdBindDescriptorSets(commandBuffer->GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->GetVkHandle(), 0, 1, &descriptorSet.descriptorSet, 0, NULL);

		// Draw debug cubes for the origin and each axis
		for (int i = 0; i < mDebugCubes.size(); i++)
		{
			DebugCube debugCube = mDebugCubes[i];

			glm::mat4 world = mat4();
			world = glm::translate(world, -debugCube.pos);
			world = glm::scale(world, glm::vec3(debugCube.size));

			PushConstantDebugBlock pushConstantBlock;
			pushConstantBlock.world = world;
			pushConstantBlock.color = debugCube.color;

			VkDeviceSize offsets[1] = { 0 };
			vkCmdPushConstants(commandBuffer->GetVkHandle(), pipelineLayout->GetVkHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushConstantBlock), &pushConstantBlock);
			vkCmdBindVertexBuffers(commandBuffer->GetVkHandle(), VERTEX_BUFFER_BIND_ID, 1, &mCubeModel->mMeshes[0]->vertices.buffer, offsets);
			vkCmdBindIndexBuffer(commandBuffer->GetVkHandle(), mCubeModel->mMeshes[0]->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer->GetVkHandle(), mCubeModel->GetNumIndices(), 1, 0, 0, 0);
		}
	}

	void RenderSystem::OnEntityAdded(const EntityCache& entityCache)
	{
		VulkanLib::StaticModel* model = mModelLoader->LoadModel(mVulkanApp->GetDeviceTmp(), entityCache.meshComponent->GetFilename());
		entityCache.meshComponent->SetModel(model);
	}

	void RenderSystem::Process()
	{
		// This will not work since we want to render all identical objects after each other
		// How the RenderSystem stores the meshes internally will not be the same as in Entity

		// It does not make sense for Update() to take an Entity as RenderSystem will not do the rendering 1..n for each Entity

		//for (auto const& entityVector : mMeshEntities)
		//{
		//	// Bind the correct pipeline

		//	for (auto const& entity : entityVector.second)
		//	{
		//		TransformComponent* transform = entity.transform;
		//		MeshComponent* meshComponent = entity.meshComponent;

		//		// Bind the correct vertex & index buffers
		//	}
		//}
	}

	void RenderSystem::AddDebugCube(vec3 pos, vec4 color, float size)
	{
		mDebugCubes.push_back(DebugCube(pos, color, size));
	}

	void RenderSystem::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{

	}
}