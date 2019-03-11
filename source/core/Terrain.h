#pragma once
#include <glm/glm.hpp>
#include <array>
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
			UNIFORM_PARAM(float, radius)
			UNIFORM_PARAM(float, strength)
			UNIFORM_PARAM(int, mode) // 0 = height, 1 = blend
			UNIFORM_PARAM(int, operation) // 0 = add, 1 = remove
		UNIFORM_BLOCK_END()

		UNIFORM_BLOCK_BEGIN(SettingsBlock)
			UNIFORM_PARAM(float, amplitudeScaling)
		UNIFORM_BLOCK_END()

		Terrain(Vk::Device* device);

		void Update();

		glm::vec2 TransformToUv(float x, float z);

		glm::vec3 GetIntersectPoint(Ray ray);
		SharedPtr<Vk::Image>& GetHeightmapImage();
		SharedPtr<Vk::Image>& GetNormalmapImage();
		SharedPtr<Vk::Image>& GetBlendmapImage();
		Vk::Mesh* GetMesh();

		float GetAmplitudeScaling();
		float GetHeight(float x, float z);
		glm::vec3 GetNormal(float x, float z);

		uint32_t GetMapResolution();
		float GetTerrainSize();

		void SetBrushBlock(const SharedPtr<BrushBlock> brushBlock);
		SharedPtr<BrushBlock> GetBrushBlock();

		void SetAmplitudeScaling(float amplitudeScaling);

		void RenderNormalmap();
		void RenderBlendmap();
		void RetrieveHeightmap();
	private:
		void EffectRecomiledCallback(std::string name);
		void GeneratePatches(float cellSize, int numCells);
		void GenerateTerrainMaps();
		void SetupHeightmapEffect();
		void SetupNormalmapEffect();
		void SetupBlendmapEffect();
		void RenderHeightmap();
		Ray LinearSearch(Ray ray);

	private:
		Vk::Device* mDevice;
		Vk::StaticModel* mQuadModel;
		float mAmplitudeScaling = 6000.0f;
		SettingsBlock settingsBlock;
		SharedPtr<BrushBlock> mBrushBlock;

		// Height & normal map members
		#define MAP_RESOLUTION 512
		SharedPtr<Vk::Effect> mHeightmapEffect;
		SharedPtr<Vk::Image> heightmapImage;
		SharedPtr<Vk::RenderTarget> heightmapRenderTarget;

		SharedPtr<Vk::Effect> mNormalmapEffect;
		SharedPtr<Vk::Image> normalImage;
		SharedPtr<Vk::RenderTarget> normalRenderTarget;

		SharedPtr<Vk::Effect> mBlendmapEffect;
		SharedPtr<Vk::Image> blendmapImage;
		SharedPtr<Vk::RenderTarget> blendmapRenderTarget;

		// Copy testing
		SharedPtr<Vk::Image> hostImage;
		std::array<float, MAP_RESOLUTION * MAP_RESOLUTION> heightmap;
		float terrainSize;

		SharedPtr<Vk::Sampler> sampler;
	};
}
