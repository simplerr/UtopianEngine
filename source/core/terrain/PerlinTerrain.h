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
		PerlinTerrain(Vk::Renderer* renderer);

		//void Update();
		virtual void AddBlock(BlockKey blockKey) override;
		virtual void GenerateBlocks() override;
		virtual float GetHeight(float x, float z) override;
	private:
		PerlinNoise<float> mPerlinNoise;
	};
}
