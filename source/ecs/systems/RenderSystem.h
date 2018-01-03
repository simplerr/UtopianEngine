#pragma once

#include <map>
#include <vector>
#include <window.h>
#include "System.h"
#include "vulkan/VulkanDebug.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/PhongEffect.h"
#include "vulkan/WaterEffect.h"
#include "vulkan/NormalDebugEffect.h"
#include "WaterRenderer.h"

class Terrain;

namespace Vulkan
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
	class Texture;
	class FrameBuffers;
	class RenderPass;
	class Sampler;
	class ScreenGui;
	class RenderTarget;
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
		RenderSystem(SystemManager* entityManager, Vulkan::Renderer* renderer, Vulkan::Camera* camera, Terrain* terrain);
		~RenderSystem();

		void OnEntityAdded(const EntityCache& entityCache);

		// Empty, everything is placed in Render()
		virtual void Process();

		// RenderSystem is treated differently from the other systems.
		// Render() wil be called from the SceneRenderer() which is required for the
		// multi pass techniques to work.
		void Render();

		virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void AddDebugCube(vec3 pos, vec4 color, float size);
	private:
		// RenderSystem should not BE a renderer but rather have one, since the usage of Vulkan should not be limited to only the ECS
		// renderer needs to be available for HUDS, debugging etc
		Vulkan::Renderer* mRenderer;
		Vulkan::ModelLoader* mModelLoader;
		Vulkan::TextureLoader* mTextureLoader;
		Vulkan::StaticModel* mCubeModel;
		Vulkan::CommandBuffer* mCommandBuffer;
		Vulkan::CommandBuffer* mTerrainCommandBuffer;
		Vulkan::CommandBuffer* mOffscreenCommandBuffer;
		Vulkan::Camera* mCamera;
		Vulkan::PhongEffect mPhongEffect;
		Vulkan::NormalDebugEffect mNormalDebugEffect;

		Terrain* mTerrain;

		std::vector<DebugCube> mDebugCubes;
	};
}