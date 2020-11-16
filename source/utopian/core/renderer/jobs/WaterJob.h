#pragma once
#include "core/renderer/jobs/BaseJob.h"
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/Texture.h"
#include "core/renderer/jobs/JobGraph.h"
#include "core/renderer/CommonBuffers.h"
#include <glm/glm.hpp>
#include <string>

namespace Utopian
{
	class WaterJob : public BaseJob
	{
	public:
		UNIFORM_BLOCK_BEGIN(FrustumPlanes)
			UNIFORM_PARAM(glm::vec4, frustumPlanes[6])
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
			UNIFORM_PARAM(glm::vec3, waterColor)
			UNIFORM_PARAM(float, waveSpeed)
			UNIFORM_PARAM(glm::vec3, foamColor)
			UNIFORM_PARAM(float, foamSpeed)
			UNIFORM_PARAM(float, distortionStrength)
			UNIFORM_PARAM(float, shorelineDepth)
			UNIFORM_PARAM(float, waveFrequency)
			UNIFORM_PARAM(float, waterSpecularity)
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
		SharedPtr<Vk::QueryPoolStatistics> mQueryPool;
		SharedPtr<Vk::Sampler> mSampler;
		FrustumPlanes mFrustumPlanesBlock;
		SettingsBlock mSettingsBlock;
		WaterParameterBlock mWaterParameterBlock;
		Vk::Mesh* mWaterMesh;
		SharedPtr<Vk::Texture> mDuDvTexture;
		SharedPtr<Vk::Texture> mNormalTexture;
		SharedPtr<Vk::Texture> mFoamMaskTexture;
		LightUniformBuffer mLightBlock;
		CascadeBlock mCascadeBlock;
		SharedPtr<Vk::Sampler> mShadowSampler;
	};
}
