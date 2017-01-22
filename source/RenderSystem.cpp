#include "RenderSystem.h"
#include "Entity.h"
#include "MeshComponent.h"
#include "TransformComponent.h"
#include "VulkanApp.h"
#include "CommandBuffer.h"
#include "Pipeline.h"
#include "PipelineLayout.h"
#include "DescriptorSet.h"

#define VERTEX_BUFFER_BIND_ID 0

namespace ECS
{
	RenderSystem::RenderSystem(VulkanLib::VulkanApp* vulkanApp)
	{
		mVulkanApp = vulkanApp;
		mModelLoader = new VulkanLib::ModelLoader();
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
		entityPair.model = mModelLoader->LoadModel(mVulkanApp->GetDeviceTmp(), entityPair.meshComponent->GetFilename());

		mMeshEntities[entityPair.meshComponent->GetPipeline()].push_back(entityPair);
	}

	void RenderSystem::Render(VulkanLib::CommandBuffer* commandBuffer, VulkanLib::Pipeline* pipeline, VulkanLib::PipelineLayout* pipelineLayout, VulkanLib::DescriptorSet& descriptorSet)
	{
		for (auto const& entityVector : mMeshEntities)
		{
			// Bind the rendering pipeline (including the shaders)
			vkCmdBindPipeline(commandBuffer->GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVkHandle());

			// Bind descriptor sets describing shader binding points (must be called after vkCmdBindPipeline!)
			vkCmdBindDescriptorSets(commandBuffer->GetVkHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->GetVkHandle(), 0, 1, &descriptorSet.descriptorSet, 0, NULL);

			for (auto const& entity : entityVector.second)
			{
				TransformComponent* transform = entity.transform;
				MeshComponent* meshComponent = entity.meshComponent;
				VulkanLib::StaticModel* model = entity.model;

				// Bind the correct vertex & index buffers
				// Push the world matrix constant
				VulkanLib::PushConstantBlock pushConstantBlock;
				pushConstantBlock.world = transform->GetWorldMatrix();
				pushConstantBlock.worldInvTranspose = transform->GetWorldInverseTransposeMatrix();
				vkCmdPushConstants(commandBuffer->GetVkHandle(), pipelineLayout->GetVkHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushConstantBlock), &pushConstantBlock);

				// Bind triangle vertices
				VkDeviceSize offsets[1] = { 0 };
				vkCmdBindVertexBuffers(commandBuffer->GetVkHandle(), VERTEX_BUFFER_BIND_ID, 1, &model->vertices.buffer, offsets);		
				vkCmdBindIndexBuffer(commandBuffer->GetVkHandle(), model->indices.buffer, 0, VK_INDEX_TYPE_UINT32);

				// Draw indexed triangle	
				vkCmdDrawIndexed(commandBuffer->GetVkHandle(), model->GetNumIndices(), 1, 0, 0, 0);
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
				VulkanLib::StaticModel* model = entity.model;

				// Bind the correct vertex & index buffers
			}
		}
	}
}