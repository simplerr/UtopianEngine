#pragma once

#include <glm/glm.hpp>
#include "utility/Common.h"
#include "utility/PerlinNoise.h"
#include "vulkan/VulkanInclude.h"
#include "core/terrain/BaseTerrain.h"

namespace Utopian
{
	struct GrassInstance 
	{
		glm::vec4 position;
		//glm::vec3 scale;
		//float textureIndex;
	};

	class PerlinTerrain : public BaseTerrain
	{
	public:
		PerlinTerrain(Vk::Renderer* renderer);

		void GenerateGrassInstances();
		Vk::Buffer* GetInstanceBuffer();
		uint32_t GetNumInstances();

		//void Update();
		virtual void AddBlock(BlockKey blockKey) override;
		virtual void GenerateBlocks() override;
		virtual float GetHeight(float x, float z) override;
	private:
		std::vector<GrassInstance> mGrassInstances;
		SharedPtr<Vk::Buffer> mInstanceBuffer;
		PerlinNoise<float> mPerlinNoise;
	};
}
