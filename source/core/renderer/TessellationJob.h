#pragma once
#include "core/renderer/BaseJob.h"
#include "vulkan/VulkanInclude.h"
#include <glm/glm.hpp>

namespace Utopian
{
	class TessellationJob : public BaseJob
	{
	public:
		UNIFORM_BLOCK_BEGIN(ViewProjection)
			UNIFORM_PARAM(glm::mat4, projection)
			UNIFORM_PARAM(glm::mat4, view)
			UNIFORM_BLOCK_END()

		UNIFORM_BLOCK_BEGIN(SettingsBlock)
			UNIFORM_PARAM(glm::vec2, viewportSize)
			UNIFORM_PARAM(float, edgeSize)
			UNIFORM_PARAM(float, tessellationFactor)
			UNIFORM_PARAM(float, amplitude)
			UNIFORM_PARAM(float, textureScaling)
		UNIFORM_BLOCK_END()

		TessellationJob(Vk::Device* device, uint32_t width, uint32_t height);
		~TessellationJob();

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(const JobInput& jobInput) override;
		void Update() override;

		SharedPtr<Vk::Image> image;
		SharedPtr<Vk::Image> depthImage;
		SharedPtr<Vk::RenderTarget> renderTarget;

	private:
		void GenerateTerrainMaps();
		void GeneratePatches(float cellSize, int numCells);
		SharedPtr<Vk::Effect> mEffect;
		Vk::StaticModel* mQuadModel;
		SharedPtr<Vk::QueryPool> mQueryPool;
		ViewProjection viewProjectionBlock;
		SettingsBlock settingsBlock;

		// Height & normal map members
		SharedPtr<Vk::Effect> mHeightmapEffect;
		SharedPtr<Vk::Effect> mNormalmapEffect;
		SharedPtr<Vk::Image> heightmapImage;
		SharedPtr<Vk::RenderTarget> heightmapRenderTarget;
		SharedPtr<Vk::Image> normalImage;
		SharedPtr<Vk::RenderTarget> normalRenderTarget;
		SharedPtr<Vk::Sampler> sampler;
	};
}
