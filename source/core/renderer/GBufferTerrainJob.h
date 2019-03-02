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

	class GBufferTerrainJob : public BaseJob
	{
	public:
		UNIFORM_BLOCK_BEGIN(ViewProjection)
			UNIFORM_PARAM(glm::mat4, projection)
			UNIFORM_PARAM(glm::mat4, view)
			UNIFORM_PARAM(glm::vec4, frustumPlanes[6])
			UNIFORM_PARAM(float, time)
		UNIFORM_BLOCK_END()

		UNIFORM_BLOCK_BEGIN(SettingsBlock)
			UNIFORM_PARAM(glm::vec2, viewportSize)
			UNIFORM_PARAM(float, edgeSize)
			UNIFORM_PARAM(float, tessellationFactor)
			UNIFORM_PARAM(float, amplitude)
			UNIFORM_PARAM(float, textureScaling)
			UNIFORM_PARAM(int, wireframe)
		UNIFORM_BLOCK_END()

		UNIFORM_BLOCK_BEGIN(BrushBlock)
			UNIFORM_PARAM(glm::vec2, brushPos)
		UNIFORM_BLOCK_END()

		GBufferTerrainJob(Vk::Device* device, const SharedPtr<Terrain>& terrain, uint32_t width, uint32_t height);
		~GBufferTerrainJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;
		void Update() override;

		SharedPtr<Vk::Image> image;
		SharedPtr<Vk::Image> depthImage;
		SharedPtr<Vk::RenderTarget> renderTarget;

	private:
		SharedPtr<Vk::Effect> mEffect;
		Vk::StaticModel* mQuadModel;
		SharedPtr<Vk::QueryPool> mQueryPool;
		SharedPtr<Vk::Sampler> sampler;
		ViewProjection viewProjectionBlock;
		SettingsBlock settingsBlock;
		SharedPtr<Terrain> mTerrain;

		Vk::TextureArray diffuseArray;
		Vk::TextureArray normalArray;
		Vk::TextureArray displacementArray;
	};
}
