#pragma once
#include <glm/glm.hpp>
#include "vulkan/VulkanInclude.h"
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Texture.h"
#include "utility/Common.h"
#include "utility/math/Ray.h"

namespace Utopian
{
	class Terrain
	{
	public:
		UNIFORM_BLOCK_BEGIN(BrushBlock)
			UNIFORM_PARAM(glm::vec2, brushPos)
		UNIFORM_BLOCK_END()

		UNIFORM_BLOCK_BEGIN(SettingsBlock)
			UNIFORM_PARAM(float, amplitudeScaling)
		UNIFORM_BLOCK_END()

		Terrain(Vk::Device* device);

		void Update();

		void EffectRecomiledCallback(std::string name);

		float GetHeight(float x, float z);
		glm::vec3 GetNormal(float x, float z);

		glm::vec2 TransformToUv(float x, float z);
		glm::vec3 GetIntersectPoint(Ray ray);

		Vk::Image* GetHeightmapImage();
		Vk::Image* GetNormalmapImage();
		Vk::Image* GetBlendmapImage();
		Vk::Mesh* GetMesh();
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
		Ray LinearSearch(Ray ray);

	private:
		Vk::Device* mDevice;
		Vk::StaticModel* mQuadModel;
		float mAmplitudeScaling = 3000.0f; // Terrain amplitude should be stored here and not as a render setting!
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
		float terrainSize;

		SharedPtr<Vk::Sampler> sampler;
	};
}
