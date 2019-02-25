#pragma once
#include "core/renderer/BaseJob.h"
#include "vulkan/VulkanInclude.h"
#include "vulkan/handles/Texture.h"
#include <glm/glm.hpp>
#include <string>

namespace Utopian
{
	class TessellationJob : public BaseJob
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

		TessellationJob(Vk::Device* device, uint32_t width, uint32_t height);
		~TessellationJob();

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(const JobInput& jobInput) override;
		void Update() override;

		void EffectRecomiledCallback(std::string name);

		SharedPtr<Vk::Image> image;
		SharedPtr<Vk::Image> depthImage;
		SharedPtr<Vk::RenderTarget> renderTarget;

	private:
		void GenerateTerrainMaps();
		void GeneratePatches(float cellSize, int numCells);
		void SetupHeightmapEffect();
		void SetupNormalmapEffect();
		void SetupBlendmapEffect();
		void SetupBlendmapBrushEffect();
		void SetupHeightmapBrushEffect();
		void RenderHeightmap();
		void RenderNormalmap();
		void RenderBlendmap();
		void RenderBlendmapBrush();
		void RenderHeightmapBrush();

		void RetrieveHeightmap();

		SharedPtr<Vk::Effect> mEffect;
		Vk::StaticModel* mQuadModel;
		SharedPtr<Vk::QueryPool> mQueryPool;
		ViewProjection viewProjectionBlock;
		SettingsBlock settingsBlock;

		// Height & normal map members
		uint32_t mapResolution = 8192;
		SharedPtr<Vk::Effect> mHeightmapEffect;
		SharedPtr<Vk::Image> heightmapImage;
		SharedPtr<Vk::RenderTarget> heightmapRenderTarget;

		SharedPtr<Vk::Effect> mNormalmapEffect;
		SharedPtr<Vk::Image> normalImage;
		SharedPtr<Vk::RenderTarget> normalRenderTarget;

		SharedPtr<Vk::Effect> mBlendmapEffect;
		SharedPtr<Vk::Image> blendmapImage;
		SharedPtr<Vk::RenderTarget> blendmapRenderTarget;

		SharedPtr<Vk::Effect> mBlendmapBrushEffect;
		SharedPtr<Vk::RenderTarget> blendmapBrushRenderTarget;
		BrushBlock brushBlock;
		glm::vec2 brushPos = glm::vec2(0.75, 0.25);

		SharedPtr<Vk::Effect> mHeightmapBrushEffect;
		SharedPtr<Vk::RenderTarget> heightmapBrushRenderTarget;

		// Copy testing
		SharedPtr<Vk::Image> hostImage;
		std::vector<float> heightmap;
		SharedPtr<Vk::Sampler> testSampler;
		float terrainSize;

		SharedPtr<Vk::Sampler> sampler;

		Vk::TextureArray diffuseArray;
		Vk::TextureArray normalArray;
		Vk::TextureArray displacementArray;
	};
}
