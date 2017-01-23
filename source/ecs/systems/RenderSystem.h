#pragma once

#include <map>
#include <vector>
#include "System.h"

namespace VulkanLib
{
	class VulkanApp;
	class ModelLoader;
	class StaticModel;
	class Pipeline;
	class PipelineLayout;
	class DescriptorSet;
	class CommandBuffer;
}

namespace ECS
{
	class MeshComponent;
	class TransformComponent;

	struct EntityPair
	{
		Entity* entity;
		MeshComponent* meshComponent;
		TransformComponent* transform;
		VulkanLib::StaticModel* model;
	};

	// RenderSystem will need to contain low level Vulkan code but by using the wrapper classes in VulkanLib as much as possible
	// It will be RenderSystem that is responsible for how objects are stored when using different amount of CPU threads

	// The render system should group meshes with the same pipeline togheter
	class RenderSystem : public System
	{
	public:
		RenderSystem(VulkanLib::VulkanApp* vulkanApp);
		~RenderSystem();

		void AddEntity(Entity* entity);

		void Render(VulkanLib::CommandBuffer* commandBuffer, std::map<int, VulkanLib::Pipeline*>& pipelines, VulkanLib::PipelineLayout* pipelineLayout, VulkanLib::DescriptorSet& descriptorSet);

		virtual void Process();
	private:
		// RenderSystem should not BE a VulkanApp but rather have one, since the usage of Vulkan should not be limited to only the ECS
		// VulkanApp needs to be available for HUDS, debugging etc
		VulkanLib::VulkanApp* mVulkanApp;
		VulkanLib::ModelLoader* mModelLoader;

		// The RenderSystem should contain a list of all loaded meshes, with only one copy of each in memory
		// What happens if a mesh changes pipeline?
		std::map<int, std::vector<EntityPair> > mMeshEntities;
	};
}