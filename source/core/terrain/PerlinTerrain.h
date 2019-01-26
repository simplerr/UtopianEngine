#pragma once

#include <glm/glm.hpp>
#include "utility/Common.h"
#include "utility/PerlinNoise.h"
#include "vulkan/VulkanInclude.h"
#include "core/terrain/BaseTerrain.h"

namespace Utopian
{
	class PerlinTerrain : public BaseTerrain
	{
	public:
		PerlinTerrain(Vk::Renderer* renderer, Camera* camera);


		void GenerateGrassInstances(glm::vec3 origin);
		Vk::Buffer* GetInstanceBuffer();
		uint32_t GetNumInstances();

		virtual void Update() override;
		virtual void AddBlock(BlockKey blockKey) override;
		virtual void GenerateBlocks() override;
		virtual float GetHeight(float x, float z) override;
		virtual glm::vec3 GetNormal(float x, float z) override;
	private:
		std::vector<GrassInstance> mGrassInstances;
		SharedPtr<Vk::Buffer> mInstanceBuffer;
		SharedPtr<PerlinNoise<float>> mPerlinNoise;
		glm::vec3 mLastGrassGenPosition;
	};
}
