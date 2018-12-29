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

namespace Utopian
{
	class Renderable;
	class Light;
	class Camera;
	class BaseJob;
	class PerlinTerrain;

	struct SceneInfo
	{
		std::vector<Renderable*> renderables;
		std::vector<Light*> lights;
		std::vector<Camera*> cameras;
		SharedPtr<PerlinTerrain> terrain;
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
		float grassViewDistance = 1000.0f;
		int blockViewDistance = 2;
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
	private:
		GBufferViewProjection viewProjectionBlock;
		SharedPtr<Vk::Sampler> sampler;
	};

	class ShadowJob : public BaseJob
	{
	public:

UNIFORM_BLOCK_BEGIN(ViewProjection)
	UNIFORM_PARAM(glm::mat4, projection)
	UNIFORM_PARAM(glm::mat4, view)
UNIFORM_BLOCK_END()

		ShadowJob(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~ShadowJob();

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(Vk::Renderer* renderer, const JobInput& jobInput) override;

		SharedPtr<Vk::RenderTarget> renderTarget;
		SharedPtr<Vk::Image> depthImage;
		SharedPtr<Vk::Image> depthImageDebug;

		SharedPtr<Vk::Effect> effect;
		ViewProjection viewProjectionBlock;
	private:
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
