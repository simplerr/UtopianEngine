#pragma once
#include <vector>
#include "vulkan/VulkanInclude.h"
#include "scene/Object.h"
#include "utility/Module.h"
#include "utility/Common.h"
#include "vulkan/PhongEffect.h"
#include "vulkan/ColorEffect.h"
#include "scene/CommonBuffers.h"
#include "vulkan/handles/DescriptorSetLayout.h"

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
		SceneRenderer(Vulkan::Renderer* renderer);
		~SceneRenderer();

		void InitShaderResources();
		void InitShader();

		void Update();
		void RenderNodes(Vulkan::CommandBuffer* commandBuffer);
		void RenderScene(Vulkan::CommandBuffer* commandBuffer);
		void Render();
		void RenderOffscreen();
		void UpdateUniformBuffers();

		void AddRenderable(Renderable* renderable);
		void AddLight(Light* light);
		void AddCamera(Vulkan::Camera* camera);
		void SetMainCamera(Vulkan::Camera* camera);

		void SetTerrain(Terrain* terrain);
		void SetClippingPlane(glm::vec4 clippingPlane);

	private:
		std::vector<Renderable*> mRenderables;
		std::vector<Light*> mLights;
		std::vector<Vulkan::Camera*> mCameras;
		Vulkan::Camera* mMainCamera;

		Vulkan::Renderer* mRenderer;
		Vulkan::CommandBuffer* mCommandBuffer;
		Vulkan::PhongEffect mPhongEffect;
		Vulkan::ColorEffect mColorEffect;
		Terrain* mTerrain;
		Vulkan::StaticModel* mCubeModel;
		WaterRenderer* mWaterRenderer;

		// Low level rendering 
		CameraUniformBuffer per_frame_vs;
		LightUniformBuffer per_frame_ps;
		FogUniformBuffer fog_ubo;
		Vulkan::DescriptorSetLayout mCommonDescriptorSetLayout;
		Vulkan::DescriptorSet* mCommonDescriptorSet;
		Vulkan::DescriptorPool* mCommonDescriptorPool;
		glm::vec4 mClippingPlane;
	};
}
