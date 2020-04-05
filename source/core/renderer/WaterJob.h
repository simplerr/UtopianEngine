#pragma once
#include "core/renderer/BaseJob.h"
#include "vulkan/VulkanInclude.h"
#include "vulkan/handles/Texture.h"
#include "core/renderer/JobGraph.h"
#include <glm/glm.hpp>
#include <string>

namespace Utopian
{
	class Terrain;

	class WaterJob : public BaseJob
	{
	public:
		UNIFORM_BLOCK_BEGIN(ViewProjection)
			UNIFORM_PARAM(glm::mat4, projection)
			UNIFORM_PARAM(glm::mat4, view)
			UNIFORM_PARAM(glm::vec4, frustumPlanes[6])
			UNIFORM_PARAM(glm::vec3, eyePos)
			UNIFORM_PARAM(float, time)
		UNIFORM_BLOCK_END()

		UNIFORM_BLOCK_BEGIN(SettingsBlock)
			UNIFORM_PARAM(glm::vec2, viewportSize)
			UNIFORM_PARAM(float, edgeSize)
			UNIFORM_PARAM(float, tessellationFactor)
			UNIFORM_PARAM(float, amplitude)
			UNIFORM_PARAM(float, textureScaling)
			UNIFORM_PARAM(float, bumpmapAmplitude)
			UNIFORM_PARAM(int, wireframe)
		UNIFORM_BLOCK_END()

        // Note: Todo: This should be shared with SkydomeJob
        UNIFORM_BLOCK_BEGIN(SkyParameterBlock)
			UNIFORM_PARAM(float, sphereRadius)
			UNIFORM_PARAM(float, inclination)
			UNIFORM_PARAM(float, azimuth)
			UNIFORM_PARAM(float, time)
			UNIFORM_PARAM(float, sunSpeed)
			UNIFORM_PARAM(glm::vec3, eyePos)
			UNIFORM_PARAM(int, onlySun)
		UNIFORM_BLOCK_END()

        // Note: Todo: Needed by ssr_vkdf.glsl. Duplicates information in ViewProjection.
        UNIFORM_BLOCK_BEGIN(SSRUniforms)
			UNIFORM_PARAM(glm::mat4, view)
			UNIFORM_PARAM(glm::mat4, projection)
		UNIFORM_BLOCK_END()

		WaterJob(Vk::Device* device, uint32_t width, uint32_t height);
		~WaterJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;
		void Update() override;

        void InitCopyPass(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer);
		void RenderCopyPass(const JobInput& jobInput);

		SharedPtr<Vk::RenderTarget> renderTarget;
	private:
        // Note: Todo: Duplicate from Terrain.h
        Vk::Mesh* GeneratePatches(float cellSize, int numCells);
    private:
		SharedPtr<Vk::Effect> mEffect;
		SharedPtr<Vk::QueryPool> mQueryPool;
		SharedPtr<Vk::Sampler> mSampler;
		ViewProjection mViewProjectionBlock;
		SettingsBlock mSettingsBlock;
		SkyParameterBlock mSkyParameterBlock;
        Vk::Mesh* mWaterMesh;

        // Todo: Likely needs to sample from the deferred job output texture, as well as writing to it.
        // Create a copy?
        SharedPtr<Vk::Image> testImage;
		SSRUniforms mSSRUniformBlock; // Remove

        // Copy pass
        // Is BlitImage better to use?
        SharedPtr<Vk::Image> originalDiffuseImage;
        SharedPtr<Vk::Image> originalDepthImage;
		SharedPtr<Vk::Effect> mCopyEffect;
		SharedPtr<Vk::RenderTarget> mCopyRenderTarget;
		SharedPtr<Vk::Semaphore> mCopyPassSemaphore;
	};
}
