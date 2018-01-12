#pragma once
#include <vector>
#include "vulkan/VulkanInclude.h"
#include "scene/Object.h"
#include "utility/Module.h"
#include "utility/Common.h"
#include "vulkan/PhongEffect.h"

class Terrain;
class WaterRenderer;

namespace Scene
{
	class Renderable;
	class Light;

	class SceneRenderer : public Module<SceneRenderer>
	{
		struct PushConstantBlock
		{
			glm::mat4 world;
			glm::mat4 worldInvTranspose;
		};

	public:
		SceneRenderer(Vulkan::Renderer* renderer, Vulkan::Camera* camera);
		~SceneRenderer();

		void InitShader();

		void Update();
		void RenderNodes(Vulkan::CommandBuffer* commandBuffer);
		void RenderScene(Vulkan::CommandBuffer* commandBuffer);
		void Render();
		void RenderOffscreen();

		void AddRenderable(Renderable* renderable);
		void AddLight(Light* light);
		void AddCamera(Vulkan::Camera* camera);

		void SetTerrain(Terrain* terrain);

	private:
		std::vector<Renderable*> mRenderables;
		std::vector<Light*> mLights;
		std::vector<Vulkan::Camera*> mCameras;

		Vulkan::Renderer* mRenderer;
		Vulkan::Camera* mCamera;
		Vulkan::CommandBuffer* mCommandBuffer;
		Vulkan::PhongEffect mPhongEffect;
		Vulkan::ScreenGui* mScreenGui;
		Terrain* mTerrain;
		WaterRenderer* mWaterRenderer;
	};
}
