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

		float GetHeight(float x, float z);
		glm::vec3 GetNormal(float x, float z);

		uint32_t GetMapResolution();

		void RenderNormalmap();
	private:
		void EffectRecomiledCallback(std::string name);
		void GeneratePatches(float cellSize, int numCells);
		void GenerateTerrainMaps();
		void SetupHeightmapEffect();
		void SetupNormalmapEffect();
		void SetupBlendmapEffect();
		void RenderHeightmap();
		void RenderBlendmap();
		void RetrieveHeightmap();
		Ray LinearSearch(Ray ray);

	private:
		Vk::Device* mDevice;
		Vk::StaticModel* mQuadModel;
		float mAmplitudeScaling = 3000.0f; // Terrain amplitude should be stored here and not as a render setting!
		SettingsBlock settingsBlock;

		// Height & normal map members
		uint32_t mapResolution = 1024;
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
		std::vector<float> heightmap;
		float terrainSize;

		SharedPtr<Vk::Sampler> sampler;
	};
}
