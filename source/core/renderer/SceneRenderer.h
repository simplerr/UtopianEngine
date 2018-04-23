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
#include "vulkan/handles/DescriptorSetLayout.h"

class Terrain;

namespace Utopian
{
	class Renderable;
	class Light;
	class WaterRenderer;

	class SceneRenderer : public Module<SceneRenderer>
	{
	public:
		/* Todo: Note the local Vertex type, should be moved to common file */
		struct QuadVertex
		{
			glm::vec3 pos;
			glm::vec2 uv;
		};

		SceneRenderer(Vk::Renderer* renderer);
		~SceneRenderer();

		void InitShaderResources();
		void InitShader();

		void Update();
		void RenderNodes(Vk::CommandBuffer* commandBuffer);
		void RenderScene(Vk::CommandBuffer* commandBuffer);
		void Render();
		void RenderOffscreen();
		void UpdateUniformBuffers();

		void AddRenderable(Renderable* renderable);
		void AddLight(Light* light);
		void AddCamera(Camera* camera);
		void SetMainCamera(Camera* camera);

		void SetTerrain(Terrain* terrain);
		void SetClippingPlane(glm::vec4 clippingPlane);

	private:
		std::vector<Renderable*> mRenderables;
		std::vector<Light*> mLights;
		std::vector<Camera*> mCameras;
		Camera* mMainCamera;

		Vk::Renderer* mRenderer;
		Vk::CommandBuffer* mCommandBuffer;
		Vk::PhongEffect mPhongEffect;
		Vk::GBufferEffect mGBufferEffect;
		Vk::ColorEffect mColorEffect;
		Vk::DeferredEffect mDeferredEffect;
		Terrain* mTerrain;
		Vk::StaticModel* mCubeModel;
		WaterRenderer* mWaterRenderer;

		// Low level rendering 
		CameraUniformBuffer per_frame_vs;
		LightUniformBuffer per_frame_ps;
		FogUniformBuffer fog_ubo;
		Vk::DescriptorSetLayout mCommonDescriptorSetLayout;
		Vk::DescriptorSet* mCommonDescriptorSet;
		Vk::DescriptorPool* mCommonDescriptorPool;
		glm::vec4 mClippingPlane;

		std::map<uint32_t, Vk::Effect*> mEffects;

		/*  Deferred rendering experimentation */
		Vk::RenderTarget* mGBufferRenderTarget;
		Vk::RenderTarget* mDeferredRenderTarget;

		struct {
			Vk::Image* position;
			Vk::Image* normal;
			Vk::Image* albedo;
			Vk::Image* depth;
		} mGBufferImages;

		struct {
			Utopian::Vk::Image* colorImage;
			Utopian::Vk::Image* depthImage;
		} mDeferredImages;

		// Todo: Move
		Vk::Buffer* mVertexBuffer;
		Vk::Buffer* mIndexBuffer;
	};
}
