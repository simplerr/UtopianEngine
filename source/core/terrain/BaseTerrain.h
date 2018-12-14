#pragma once
#include <map>
#include <glm/glm.hpp>
#include "utility/Common.h"
#include "vulkan/VulkanInclude.h"
#include "core/renderer/Renderable.h"

namespace Utopian
{
	struct BlockKey
	{
		BlockKey(int32_t _x, int32_t _y, int32_t _z)
			: x(_x), y(_y), z(_z) {
		}

		int32_t x, y, z;
	};

	bool operator<(BlockKey const& a, BlockKey const& b);

	struct Block2
	{
		Block2(glm::vec3 pos, glm::vec3 col) {
			position = pos;
			color = col;
			generated = false;
			modified = false;
			visible = false;

		}

		SharedPtr<Renderable> renderable;
		glm::vec3 position;
		glm::vec3 color;
		bool generated;
		bool modified;
		bool visible;
	};

	class BaseTerrain
	{
	public:
		BaseTerrain(Vk::Renderer* renderer);

		virtual void Update();
		virtual void GenerateBlocks() = 0;
		virtual void AddBlock(BlockKey blockKey) = 0;
		virtual float GetHeight(float x, float z) = 0;

		std::map<BlockKey, SharedPtr<Block2>>& GetBlocks();

	protected:
		std::map<BlockKey, SharedPtr<Block2>> mBlockList;
		Vk::Renderer* mRenderer;
		const int32_t mVoxelsInBlock = 32;
		const int32_t mVoxelSize = 40;
		const int32_t mViewDistance = 2;
	};
}
