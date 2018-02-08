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

namespace Utopian
{
	class Renderable;
	class Light;

	class SceneRenderer : public Module<SceneRenderer>
	{
	public:
		SceneRenderer(Utopian::Vk::Renderer* renderer);
		~SceneRenderer();

		void InitShaderResources();
		void InitShader();

		void Update();
		void RenderNodes(Utopian::Vk::CommandBuffer* commandBuffer);
		void RenderScene(Utopian::Vk::CommandBuffer* commandBuffer);
		void Render();
		void RenderOffscreen();
		void UpdateUniformBuffers();

		void AddRenderable(Renderable* renderable);
		void AddLight(Light* light);
		void AddCamera(Utopian::Vk::Camera* camera);
		void SetMainCamera(Utopian::Vk::Camera* camera);

		void SetTerrain(Terrain* terrain);
		void SetClippingPlane(glm::vec4 clippingPlane);

	private:
		std::vector<Renderable*> mRenderables;
		std::vector<Light*> mLights;
		std::vector<Utopian::Vk::Camera*> mCameras;
		Utopian::Vk::Camera* mMainCamera;

		Utopian::Vk::Renderer* mRenderer;
		Utopian::Vk::CommandBuffer* mCommandBuffer;
		Utopian::Vk::PhongEffect mPhongEffect;
		Utopian::Vk::ColorEffect mColorEffect;
		Terrain* mTerrain;
		Utopian::Vk::StaticModel* mCubeModel;
		WaterRenderer* mWaterRenderer;

		// Low level rendering 
		CameraUniformBuffer per_frame_vs;
		LightUniformBuffer per_frame_ps;
		FogUniformBuffer fog_ubo;
		Utopian::Vk::DescriptorSetLayout mCommonDescriptorSetLayout;
		Utopian::Vk::DescriptorSet* mCommonDescriptorSet;
		Utopian::Vk::DescriptorPool* mCommonDescriptorPool;
		glm::vec4 mClippingPlane;
	};
}
