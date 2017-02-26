#pragma once

#include <map>
#include <vector>
#include <window.h>
#include "System.h"
#include "vulkan/VulkanDebug.h"

namespace VulkanLib
{
	class Renderer;
	class Camera;
	class ModelLoader;
	class TextureLoader;
	class StaticModel;
	class Pipeline;
	class PipelineLayout;
	class DescriptorSet;
	class CommandBuffer;
	class CubeMesh;
	class VertexDescription;

	class GeometryUniformBuffer : public UniformBuffer
	{
	public:
		virtual void UpdateMemory(VkDevice device)
		{
			// Map uniform buffer and update it
			uint8_t *data1;
			VulkanLib::VulkanDebug::ErrorCheck(vkMapMemory(device, mMemory, 0, sizeof(data), 0, (void **)&data1));
			memcpy(data1, &data, sizeof(data));
			vkUnmapMemory(device, mMemory);
		}

		virtual int GetSize()
		{
			return sizeof(data);
		}

		// Public data members
		struct {
			glm::mat4 projection;
			glm::mat4 view;
		} data;
	};
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
		RenderSystem(EntityManager* entityManager, VulkanLib::Renderer* renderer, VulkanLib::Camera* cameraj);
		~RenderSystem();

		void OnEntityAdded(const EntityCache& entityCache);
		virtual void Process();

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void AddDebugCube(vec3 pos, vec4 color, float size);
	private:
		// RenderSystem should not BE a renderer but rather have one, since the usage of Vulkan should not be limited to only the ECS
		// renderer needs to be available for HUDS, debugging etc
		VulkanLib::Renderer* mRenderer;
		VulkanLib::ModelLoader* mModelLoader;
		VulkanLib::TextureLoader* mTextureLoader;
		VulkanLib::StaticModel* mCubeModel;
		VulkanLib::CommandBuffer* mCommandBuffer;
		VulkanLib::Camera* mCamera;

		std::vector<DebugCube> mDebugCubes;

		// Geometry shader stuff
		VulkanLib::DescriptorPool* mDescriptorPool;
		VulkanLib::DescriptorSetLayout* mDescriptorSetLayout;
		VulkanLib::DescriptorSet* mDescriptorSet;
		VulkanLib::Pipeline* mGeometryPipeline;
		VulkanLib::PipelineLayout* mPipelineLayout;
		VulkanLib::GeometryUniformBuffer  mUniformBuffer;

		struct PushConstantBlock {
			mat4 world;
			mat4 worldInvTranspose;
		};
	};
}