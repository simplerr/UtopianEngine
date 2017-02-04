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


#define VERTEX_BUFFER_BIND_ID 0

namespace ECS
{
	RenderSystem::RenderSystem(VulkanLib::VulkanApp* vulkanApp)
	{
		mVulkanApp = vulkanApp;
		mModelLoader = new VulkanLib::ModelLoader();

		mCubeModel = mModelLoader->LoadDebugBox(vulkanApp->GetDeviceTmp());
	}

	RenderSystem::~RenderSystem()
	{
		mModelLoader->CleanupModels(mVulkanApp->GetDevice());
		delete mModelLoader;
	}

	void RenderSystem::AddEntity(Entity* entity)
	{
		EntityPair entityPair;
		entityPair.entity = entity;
		entityPair.transform = dynamic_cast<TransformComponent*>(entity->GetComponent(TRANSFORM_COMPONENT));
		entityPair.meshComponent = dynamic_cast<MeshComponent*>(entity->GetComponent(MESH_COMPONENT));
		VulkanLib::StaticModel* model = mModelLoader->LoadModel(mVulkanApp->GetDeviceTmp(), entityPair.meshComponent->GetFilename()); 
		entityPair.meshComponent->SetModel(model);

		mMeshEntities[entityPair.meshComponent->GetPipeline()].push_back(entityPair);
	}

	void RenderSystem::Render(VulkanLib::CommandBuffer* commandBuffer, std::map<int, VulkanLib::Pipeline*>& pipelines, VulkanLib::PipelineLayout* pipelineLayout, VulkanLib::DescriptorSet& descriptorSet)
	{
		// Draw all meshes
		for (auto const& entityVector : mMeshEntities)
		{
			// Bind the rendering pipeline (including the shaders)
			vkCmdBindPipeline(commandBuffer->GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[entityVector.first]->GetVkHandle());

			// Bind descriptor sets describing shader binding points (must be called after vkCmdBindPipeline!)
			vkCmdBindDescriptorSets(commandBuffer->GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->GetVkHandle(), 0, 1, &descriptorSet.descriptorSet, 0, NULL);

			for (auto const& entity : entityVector.second)
			{
				TransformComponent* transform = entity.transform;
				MeshComponent* meshComponent = entity.meshComponent;
				VulkanLib::StaticModel* model = entity.meshComponent->GetModel();

				// Bind the correct vertex & index buffers
				// Push the world matrix constant
				VulkanLib::PushConstantBlock pushConstantBlock;
				pushConstantBlock.world = transform->GetWorldMatrix();
				pushConstantBlock.worldInvTranspose = transform->GetWorldInverseTransposeMatrix();
				vkCmdPushConstants(commandBuffer->GetVkHandle(), pipelineLayout->GetVkHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushConstantBlock), &pushConstantBlock);

				// Bind triangle vertices
				VkDeviceSize offsets[1] = { 0 };
				vkCmdBindVertexBuffers(commandBuffer->GetVkHandle(), VERTEX_BUFFER_BIND_ID, 1, &model->mMeshes[0]->vertices.buffer, offsets);		
				vkCmdBindIndexBuffer(commandBuffer->GetVkHandle(), model->mMeshes[0]->indices.buffer, 0, VK_INDEX_TYPE_UINT32);

				// Draw indexed triangle	
				vkCmdDrawIndexed(commandBuffer->GetVkHandle(), model->GetNumIndices(), 1, 0, 0, 0);

				// Draw debug bounding box
				VulkanLib::BoundingBox meshBoundingBox = entity.meshComponent->GetBoundingBox();
				meshBoundingBox.Update(entity.transform->GetWorldMatrix()); 

				mat4 world;
				float width = meshBoundingBox.GetWidth();
				float height = meshBoundingBox.GetHeight();
				float depth = meshBoundingBox.GetDepth();
				world = glm::scale(world, glm::vec3(width, height, depth));
				world = glm::translate(world, transform->GetPosition());

				pushConstantBlock.world = world;

				vkCmdPushConstants(commandBuffer->GetVkHandle(), pipelineLayout->GetVkHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushConstantBlock), &pushConstantBlock);
				vkCmdBindVertexBuffers(commandBuffer->GetVkHandle(), VERTEX_BUFFER_BIND_ID, 1, &mCubeModel->mMeshes[0]->vertices.buffer, offsets);
				vkCmdBindIndexBuffer(commandBuffer->GetVkHandle(), mCubeModel->mMeshes[0]->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(commandBuffer->GetVkHandle(), mCubeModel->GetNumIndices(), 1, 0, 0, 0);
			}
		}
	}

	void RenderSystem::Process()
	{
		// This will not work since we want to render all identical objects after each other
		// How the RenderSystem stores the meshes internally will not be the same as in Entity

		// It does not make sense for Update() to take an Entity as RenderSystem will not do the rendering 1..n for each Entity

		for (auto const& entityVector : mMeshEntities)
		{
			// Bind the correct pipeline

			for (auto const& entity : entityVector.second)
			{
				TransformComponent* transform = entity.transform;
				MeshComponent* meshComponent = entity.meshComponent;

				// Bind the correct vertex & index buffers
			}
		}
	}
}