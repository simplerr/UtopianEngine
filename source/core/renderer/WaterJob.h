#pragma once
#include "core/renderer/BaseJob.h"
#include "vulkan/VulkanInclude.h"
#include "vulkan/handles/Texture.h"
#include "core/renderer/JobGraph.h"
#include "core/CommonBuffers.h"
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

        UNIFORM_BLOCK_BEGIN(WaterParameterBlock)
			UNIFORM_PARAM(float, time)
		UNIFORM_BLOCK_END()

		WaterJob(Vk::Device* device, uint32_t width, uint32_t height);
		~WaterJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;
		void Update() override;

		SharedPtr<Vk::RenderTarget> renderTarget;
        SharedPtr<Vk::Image> distortionImage;
	private:
        // Note: Todo: Duplicate from Terrain.h
        Vk::Mesh* GeneratePatches(float cellSize, int numCells);
    private:
		SharedPtr<Vk::Effect> mEffect;
		SharedPtr<Vk::QueryPool> mQueryPool;
		SharedPtr<Vk::Sampler> mSampler;
		ViewProjection mViewProjectionBlock;
		SettingsBlock mSettingsBlock;
		WaterParameterBlock mWaterParameterBlock;
        Vk::Mesh* mWaterMesh;
		Vk::Texture* mDuDvTexture;
		Vk::Texture* mNormalTexture;
		LightUniformBuffer mLightBlock;
		CascadeBlock mCascadeBlock;
		SharedPtr<Vk::Sampler> mShadowSampler;
	};
}
