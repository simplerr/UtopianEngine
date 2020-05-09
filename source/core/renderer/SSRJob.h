#pragma once

#include "core/renderer/BaseJob.h"
#include "vulkan/SSAOEffect.h"

namespace Utopian
{
	class SSRJob : public BaseJob
	{
	public:

		UNIFORM_BLOCK_BEGIN(SSRUniforms)
			UNIFORM_PARAM(glm::mat4, view)
			UNIFORM_PARAM(glm::mat4, projection)
		UNIFORM_BLOCK_END()

		UNIFORM_BLOCK_BEGIN(BlurSettingsBlock)
			UNIFORM_PARAM(int, blurRange)
		UNIFORM_BLOCK_END()

		// Note: Todo: This should be shared with SkydomeJob
        UNIFORM_BLOCK_BEGIN(SkyParameterBlock)
			UNIFORM_PARAM(glm::vec3, eyePos)
			UNIFORM_PARAM(float, sphereRadius)
			UNIFORM_PARAM(float, inclination)
			UNIFORM_PARAM(float, azimuth)
			UNIFORM_PARAM(float, time)
			UNIFORM_PARAM(float, sunSpeed)
			UNIFORM_PARAM(int, onlySun)
		UNIFORM_BLOCK_END()

		UNIFORM_BLOCK_BEGIN(ReflectionSettingsBlock)
			UNIFORM_PARAM(int, ssrEnabled)
			UNIFORM_PARAM(int, skyboxReflections)
		UNIFORM_BLOCK_END()

		UNIFORM_BLOCK_BEGIN(SSRSettingsBlock)
			UNIFORM_PARAM(glm::mat4, _CameraProjectionMatrix)        // projection matrix that maps to screen pixels (not NDC)
			UNIFORM_PARAM(glm::mat4, _CameraInverseProjectionMatrix) // inverse projection matrix (NDC to camera space)
			UNIFORM_PARAM(glm::mat4, _NormalMatrix)
			UNIFORM_PARAM(glm::mat4, _ViewMatrix)
			UNIFORM_PARAM(glm::vec2, _RenderBufferSize)
			UNIFORM_PARAM(glm::vec2, _OneDividedByRenderBufferSize)  // Optimization: removes 2 divisions every itteration
			UNIFORM_PARAM(float, _Iterations)                        // maximum ray iterations
			UNIFORM_PARAM(float, _BinarySearchIterations)            // maximum binary search refinement iterations
			UNIFORM_PARAM(float, _PixelZSize)                        // Z size in camera space of a pixel in the depth buffer
			UNIFORM_PARAM(float, _PixelStride)                       // number of pixels per ray step close to camera
			UNIFORM_PARAM(float, _PixelStrideZCuttoff)               // ray origin Z at this distance will have a pixel stride of 1.0
			UNIFORM_PARAM(float, _MaxRayDistance)                    // maximum distance of a ray
			UNIFORM_PARAM(float, _ScreenEdgeFadeStart)               // distance to screen edge that ray hits will start to fade (0.0 -> 1.0)
			UNIFORM_PARAM(float, _EyeFadeStart)                      // ray direction's Z that ray hits will start to fade (0.0 -> 1.0)
			UNIFORM_PARAM(float, _EyeFadeEnd)                        // ray direction's Z that ray hits will be cut (0.0 -> 1.0)
		UNIFORM_BLOCK_END()
  
		SSRJob(Vk::Device* device, uint32_t width, uint32_t height);
		~SSRJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;
		void Update() override;

		SharedPtr<Vk::Image> ssrBlurImage;
		SharedPtr<Vk::Image> ssrImage;

		// Debug images
		SharedPtr<Vk::Image> rayOriginImage;
		SharedPtr<Vk::Image> rayEndImage;
		SharedPtr<Vk::Image> miscDebugImage;

	private:
		void InitTracePass(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer);
		void InitTracePassKode80(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer);
		void InitBlurPass(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer);

		void RenderTracePass(const JobInput& jobInput);
		void RenderTracePassKode80(const JobInput& jobInput);
		void RenderBlurPass(const JobInput& jobInput);

		// Two pass effect
		SharedPtr<Vk::Effect> mTraceSSREffect;
		SharedPtr<Vk::Effect> mBlurSSREffect;
		SharedPtr<Vk::Semaphore> mTracePassSemaphore;
		SharedPtr<Vk::Semaphore> mBlurPassSemaphore;

		SharedPtr<Vk::RenderTarget> mTraceRenderTarget;
		SharedPtr<Vk::RenderTarget> mBlurRenderTarget;
		SSRUniforms mUniformBlock;
		SkyParameterBlock mSkyParameterBlock;
		ReflectionSettingsBlock mReflectionSettingsBlock;

		SSRSettingsBlock mSSRSettingsBlock;
	};
}