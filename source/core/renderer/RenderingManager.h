#pragma once
#include <vector>
#include "vulkan/VulkanInclude.h"
#include "core/Object.h"
#include "utility/Module.h"
#include "utility/Common.h"
#include "utility/Timer.h"
#include "core/CommonBuffers.h"
#include "core/renderer/BaseJob.h"
#include "core/renderer/RenderSettings.h"
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
			GRASS_INDEX,
			SKYBOX_INDEX,
			SUN_SHAFT_INDEX,
			DEBUG_INDEX,
		};

		RenderingManager(Vk::VulkanApp* vulkanApp);
		~RenderingManager();

		void PostWorldInit();
		void InitShaderResources();

		void Update();
		void UpdateUi();
		void UpdateCascades();
		void RenderNodes(Vk::CommandBuffer* commandBuffer);
		void RenderScene(Vk::CommandBuffer* commandBuffer);
		void Render();
		void RenderOffscreen();
		void UpdateUniformBuffers();

		void AddRenderable(Renderable* renderable);
		void AddLight(Light* light);
		void AddCamera(Camera* camera);

		// Instancing experimentation
		void AddInstancedAsset(uint32_t assetId, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
		void BuildAllInstances();
		void ClearInstanceGroups();

		void RemoveRenderable(Renderable* renderable);
		void RemoveLight(Light* light);
		void RemoveCamera(Camera* camera);

		void SetMainCamera(Camera* camera);

		void SetTerrain(Terrain* terrain);
		void SetClippingPlane(glm::vec4 clippingPlane);

		void AddJob(BaseJob* job);

		Camera* GetMainCamera();

		BaseTerrain* GetTerrain();
		RenderingSettings& GetRenderingSettings();
	private:
		SceneInfo mSceneInfo;
		Camera* mMainCamera;
		uint32_t mNextNodeId;

		Vk::VulkanApp* mVulkanApp;
		Vk::Device* mDevice;
		Vk::CommandBuffer* mCommandBuffer;
		SharedPtr<Vk::PhongEffect> mPhongEffect;
		Terrain* mTerrain;
		WaterRenderer* mWaterRenderer;

		// TODO: Move these
		CameraUniformBuffer per_frame_vs;
		LightUniformBuffer per_frame_ps;
		SettingsUniformBuffer fog_ubo;

		Vk::DescriptorSetLayout mCommonDescriptorSetLayout;
		Vk::DescriptorSet* mCommonDescriptorSet;
		Vk::DescriptorPool* mCommonDescriptorPool;
		glm::vec4 mClippingPlane;

		/*  Deferred rendering experimentation */
		std::vector<BaseJob*> mJobs;
		RenderingSettings mRenderingSettings;
	};

	RenderingManager& gRenderingManager();
}
