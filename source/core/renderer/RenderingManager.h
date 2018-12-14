#pragma once
#include <vector>
#include "vulkan/VulkanInclude.h"
#include "core/Object.h"
#include "utility/Module.h"
#include "utility/Common.h"
#include "vulkan/PhongEffect.h"
#include "vulkan/GBufferEffect.h"
#include "vulkan/DeferredEffect.h"
#include "vulkan/ColorEffect.h"
#include "core/CommonBuffers.h"
#include "core/renderer/SceneJobs.h"
#include "vulkan/handles/DescriptorSetLayout.h"

class Terrain;

namespace Utopian
{
	class Renderable;
	class Light;
	class WaterRenderer;
	class BaseTerrain;

	namespace Vk
	{
		class BasicRenderTarget;
	}

	class RenderingManager : public Module<RenderingManager>
	{
	public:
		enum JobIndex
		{
			GBUFFER_INDEX = 0,
			SSAO_INDEX,
			BLUR_INDEX,
			SHADOW_INDEX,
			DEFERRED_INDEX,
			SKYBOX_INDEX,
			DEBUG_INDEX,
		};

		RenderingManager(Vk::Renderer* renderer);
		~RenderingManager();

		void InitShaderResources();
		void InitShader();

		void Update();
		void UpdateUi();
		void RenderNodes(Vk::CommandBuffer* commandBuffer);
		void RenderScene(Vk::CommandBuffer* commandBuffer);
		void Render();
		void RenderOffscreen();
		void UpdateUniformBuffers();

		void AddRenderable(Renderable* renderable);
		void AddLight(Light* light);
		void AddCamera(Camera* camera);

		void RemoveRenderable(Renderable* renderable);
		void RemoveLight(Light* light);
		void RemoveCamera(Camera* camera);

		void SetMainCamera(Camera* camera);

		void SetTerrain(Terrain* terrain);
		void SetClippingPlane(glm::vec4 clippingPlane);

		void AddJob(BaseJob* job);

		BaseTerrain* GetTerrain();
	private:
		SceneInfo mSceneInfo;
		Camera* mMainCamera;
		uint32_t mNextNodeId;

		Vk::Renderer* mRenderer;
		Vk::CommandBuffer* mCommandBuffer;
		Vk::PhongEffect mPhongEffect;
		Terrain* mTerrain;
		WaterRenderer* mWaterRenderer;

		// TODO: Move these
		CameraUniformBuffer per_frame_vs;
		LightUniformBuffer per_frame_ps;
		FogUniformBuffer fog_ubo;

		Vk::DescriptorSetLayout mCommonDescriptorSetLayout;
		Vk::DescriptorSet* mCommonDescriptorSet;
		Vk::DescriptorPool* mCommonDescriptorPool;
		glm::vec4 mClippingPlane;

		std::map<uint32_t, Vk::EffectLegacy*> mEffects;

		/*  Deferred rendering experimentation */
		std::vector<BaseJob*> mJobs;
		RenderingSettings mRenderingSettings;
	};
}
