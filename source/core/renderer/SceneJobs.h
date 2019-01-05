#pragma once

#include <vector>
#include "vulkan/GBufferEffect.h"
#include "vulkan/DeferredEffect.h"
#include "vulkan/SSAOEffect.h"
#include "vulkan/BlurEffect.h"
#include "vulkan/ColorEffect.h"
#include "vulkan/SkyboxEffect.h"
#include "vulkan/NormalDebugEffect.h"
#include "vulkan/VulkanInclude.h"
#include "utility/Common.h"
#include "vulkan/ShaderBuffer.h"

#define SHADOW_MAP_CASCADE_COUNT 4

namespace Utopian
{
	class Renderable;
	class Light;
	class Camera;
	class BaseJob;
	class PerlinTerrain;	
	
	/*
		Instancing data
		Todo: Move
	*/
	struct InstanceData
	{
		glm::mat4 world;   
	};

	class InstanceGroup
	{
	public:
		InstanceGroup(uint32_t assetId);
		
		void AddInstance(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
		void ClearInstances();
		void BuildBuffer(Vk::Renderer* renderer);

		uint32_t GetAssetId();
		uint32_t GetNumInstances();
		Vk::Buffer* GetBuffer();
		Vk::StaticModel* GetModel();

	private:
		SharedPtr<Vk::Buffer> mInstanceBuffer;
		Vk::StaticModel* mModel;
		std::vector<InstanceData> mInstances;
		uint32_t mAssetId;
	};

	class Cascade
	{
	public:
		float splitDepth;
		glm::mat4 viewProjMatrix;
	};

	struct SceneInfo
	{
		std::vector<Renderable*> renderables;
		std::vector<Light*> lights;
		std::vector<Camera*> cameras;
		SharedPtr<PerlinTerrain> terrain;
		std::vector<SharedPtr<InstanceGroup>> instanceGroups;
		std::array<Cascade, SHADOW_MAP_CASCADE_COUNT> cascades;

		// The light that will cast shadows
		// Currently assumes that there only is one directional light in the scene
		Light* directionalLight;

		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
		glm::vec3 eyePos;
	};

	struct RenderingSettings
	{
		glm::vec4 fogColor;
		bool deferredPipeline;
		float fogStart;
		float fogDistance;
		float ssaoRadius = 6.0f;
		float ssaoBias = 0.0f;
		int blurRadius = 2;
		float grassViewDistance = 0*1800.0f;
		int blockViewDistance = 2;
		int shadowSampleSize = 1;
		bool cascadeColorDebug = 0;
		float cascadeSplitLambda = 0.927f;
		float nearPlane = 1.0f;
		float farPlane = 25600.0f;
		float sunSpeed = 1.0f;
		float sunInclination = 45.0f;
		float sunAzimuth = 0.0f;
	};

	struct JobInput
	{
		JobInput(const SceneInfo& sceneInfo, const std::vector<BaseJob*>& jobs, const RenderingSettings& renderingSettings) 
			: sceneInfo(sceneInfo), jobs(jobs) , renderingSettings(renderingSettings) {

		}

		const SceneInfo& sceneInfo;
		const std::vector<BaseJob*>& jobs;
		const RenderingSettings& renderingSettings;
	};

	class BaseJob
	{
	public:
		BaseJob(Vk::Renderer* renderer, uint32_t width, uint32_t height) {
			mRenderer = renderer;
			mWidth = width;
			mHeight = height;
		}

		virtual ~BaseJob() {};

		/*
		 * If a job needs to query information from another job that's already added
		   it should be done inside of this function.
		*/
		virtual void Init(const std::vector<BaseJob*>& jobs) = 0;

		virtual void Render(Vk::Renderer* renderer, const JobInput& jobInput) = 0;
	protected:
		Vk::Renderer* mRenderer;
		uint32_t mWidth;
		uint32_t mHeight;
	};

	class GBufferJob : public BaseJob
	{
	public:
		UNIFORM_BLOCK_BEGIN(GBufferViewProjection)
			UNIFORM_PARAM(glm::mat4, projection)
			UNIFORM_PARAM(glm::mat4, view)
			UNIFORM_BLOCK_END()

		GBufferJob(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~GBufferJob();

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(Vk::Renderer* renderer, const JobInput& jobInput) override;

		SharedPtr<Vk::Image> positionImage;
		SharedPtr<Vk::Image> normalImage;
		SharedPtr<Vk::Image> normalViewImage; // Normals in view space
		SharedPtr<Vk::Image> albedoImage;
		SharedPtr<Vk::Image> depthImage;
		SharedPtr<Vk::RenderTarget> renderTarget;

		SharedPtr<Vk::GBufferEffect> mGBufferEffect;
		SharedPtr<Vk::GBufferEffect> mGBufferEffectWireframe;
		SharedPtr<Vk::Effect> mGBufferEffectTerrain;
		SharedPtr<Vk::Effect> mGBufferEffectInstanced;
	private:
		GBufferViewProjection viewProjectionBlock;
		SharedPtr<Vk::Sampler> sampler;
	};

	class ShadowJob : public BaseJob
	{
	public:

UNIFORM_BLOCK_BEGIN(CascadeTransforms)
	UNIFORM_PARAM(glm::mat4, viewProjection[SHADOW_MAP_CASCADE_COUNT])
UNIFORM_BLOCK_END()

		struct CascadePushConst
		{
			CascadePushConst(glm::mat4 _world, uint32_t _cascadeIndex) {

				world = _world;
				// Note: This needs to be done to have the physical world match the rendered world.
				// See https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/ for more information.
				world[3][0] = -world[3][0];
				world[3][1] = -world[3][1];
				world[3][2] = -world[3][2];

				cascadeIndex = _cascadeIndex;
			}

			glm::mat4 world;
			uint32_t cascadeIndex;
		};

		ShadowJob(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~ShadowJob();

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(Vk::Renderer* renderer, const JobInput& jobInput) override;

		SharedPtr<Vk::RenderTarget> renderTarget;
		SharedPtr<Vk::Image> depthColorImage;
		SharedPtr<Vk::Image> depthImage;

		SharedPtr<Vk::Effect> effect;
		SharedPtr<Vk::Effect> effectInstanced;
		CascadeTransforms cascadeTransforms;
	private:
		const uint32_t SHADOWMAP_DIMENSION = 4096;
		std::vector<SharedPtr<Vk::FrameBuffers>> mFrameBuffers;
	};

	class DeferredJob : public BaseJob
	{
	public:
		DeferredJob(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~DeferredJob();

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(Vk::Renderer* renderer, const JobInput& jobInput) override;

		SharedPtr<Vk::BasicRenderTarget> renderTarget;
		SharedPtr<Vk::Sampler> depthSampler;
		SharedPtr<Vk::DeferredEffect> effect;
	private:
		SharedPtr<Vk::ScreenQuad> mScreenQuad;
	};

	class SSAOJob : public BaseJob
	{
	public:
		SSAOJob(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~SSAOJob();

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(Vk::Renderer* renderer, const JobInput& jobInput) override;

		SharedPtr<Vk::Image> ssaoImage;
		SharedPtr<Vk::RenderTarget> renderTarget;

		SharedPtr<Vk::SSAOEffect> effect;
	private:
		void CreateKernelSamples();
	};

	class BlurJob : public BaseJob
	{
	public:
		BlurJob(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~BlurJob();

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(Vk::Renderer* renderer, const JobInput& jobInput) override;

		SharedPtr<Vk::Image> blurImage;
		SharedPtr<Vk::RenderTarget> renderTarget;

		SharedPtr<Vk::BlurEffect> effect;
	private:
	};

	class SkyboxJob : public BaseJob
	{
	public:
		SkyboxJob(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~SkyboxJob();

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(Vk::Renderer* renderer, const JobInput& jobInput) override;

		SharedPtr<Vk::CubeMapTexture> skybox;
		SharedPtr<Vk::RenderTarget> renderTarget;

		SharedPtr<Vk::SkyboxEffect> effect;
	private:
		Vk::StaticModel* mCubeModel;
	};

	class SkydomeJob : public BaseJob
	{
	public:

UNIFORM_BLOCK_BEGIN(ViewProjection)
	UNIFORM_PARAM(glm::mat4, projection)
	UNIFORM_PARAM(glm::mat4, view)
	UNIFORM_PARAM(glm::mat4, world)
UNIFORM_BLOCK_END()

UNIFORM_BLOCK_BEGIN(ParameterBlock)
	UNIFORM_PARAM(float, sphereRadius)
	UNIFORM_PARAM(float, inclination)
	UNIFORM_PARAM(float, azimuth)
	UNIFORM_PARAM(float, time)
	UNIFORM_PARAM(float, sunSpeed)
UNIFORM_BLOCK_END()

		SkydomeJob(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~SkydomeJob();

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(Vk::Renderer* renderer, const JobInput& jobInput) override;

		SharedPtr<Vk::RenderTarget> renderTarget;

		SharedPtr<Vk::Effect> effect;
	private:
		ViewProjection viewProjectionBlock;
		ParameterBlock parameterBlock;
		Vk::StaticModel* mSkydomeModel;
		float sunAzimuth;
	};

	class DebugJob : public BaseJob
	{
	public:
		DebugJob(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~DebugJob();

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(Vk::Renderer* renderer, const JobInput& jobInput) override;

		SharedPtr<Vk::RenderTarget> renderTarget;

		SharedPtr<Vk::ColorEffect> colorEffect;
		SharedPtr<Vk::ColorEffect> colorEffectWireframe;
		SharedPtr<Vk::NormalDebugEffect> normalEffect;
	private:
		Vk::StaticModel* mCubeModel;
	};

	class GrassJob : public BaseJob
	{
	public:

UNIFORM_BLOCK_BEGIN(ViewProjection)
	UNIFORM_PARAM(glm::mat4, projection)
	UNIFORM_PARAM(glm::mat4, view)
	UNIFORM_PARAM(glm::vec4, eyePos)
	UNIFORM_PARAM(float, grassViewDistance)
UNIFORM_BLOCK_END()

		GrassJob(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~GrassJob();

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(Vk::Renderer* renderer, const JobInput& jobInput) override;

		SharedPtr<Vk::RenderTarget> renderTarget;
		SharedPtr<Vk::Effect> effect;
		SharedPtr<Vk::Sampler> sampler;
		ViewProjection viewProjectionBlock;
	private:
	};
}
