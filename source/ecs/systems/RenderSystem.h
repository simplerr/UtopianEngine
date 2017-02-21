#pragma once

#include <map>
#include <vector>
#include <window.h>
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
	class CubeMesh;
}

namespace ECS
{
	class MeshComponent;
	class TransformComponent;

	struct PushConstantDebugBlock {
		mat4 world;
		vec4 color;
	};

	struct DebugCube
	{
		DebugCube(vec3 pos, vec4 color, float size) {
			this->pos = pos;
			this->color = color;
			this->size = size;
		}

		vec3 pos;
		vec4 color;
		float size;
	};

	// RenderSystem will need to contain low level Vulkan code but by using the wrapper classes in VulkanLib as much as possible
	// It will be RenderSystem that is responsible for how objects are stored when using different amount of CPU threads

	// The render system should group meshes with the same pipeline togheter
	class RenderSystem : public System
	{
	public:
		RenderSystem(EntityManager* entityManager, VulkanLib::VulkanApp* vulkanApp);
		~RenderSystem();

		void OnEntityAdded(const EntityCache& entityCache);
		virtual void Process();

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void AddDebugCube(vec3 pos, vec4 color, float size);
	private:
		// RenderSystem should not BE a VulkanApp but rather have one, since the usage of Vulkan should not be limited to only the ECS
		// VulkanApp needs to be available for HUDS, debugging etc
		VulkanLib::VulkanApp* mVulkanApp;
		VulkanLib::ModelLoader* mModelLoader;
		VulkanLib::StaticModel* mCubeModel;
		VulkanLib::CommandBuffer* mCommandBuffer;

		std::vector<DebugCube> mDebugCubes;
	};
}