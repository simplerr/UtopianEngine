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

	struct GrassInstance 
	{
		glm::vec4 position;
		glm::vec3 color;
		//glm::vec3 scale;
		//float textureIndex;
	};

	class BaseTerrain;

	bool operator<(BlockKey const& a, BlockKey const& b);

	class Block2
	{
	public:
		Block2(glm::vec3 pos, glm::vec3 col) {
			position = pos;
			color = col;
			generated = false;
			modified = false;
			visible = false;
		}

		void GenerateGrassInstances(Vk::Renderer* renderer, BaseTerrain* terrain, int32_t cellsInBlock, int32_t cellSize);

		SharedPtr<Renderable> renderable;
		std::vector<GrassInstance> grassInstances;
		SharedPtr<Vk::Buffer> instanceBuffer;
		glm::vec3 position;
		glm::vec3 color;
		bool generated;
		bool modified;
		bool visible;
	private:
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
		void ClearBlocks();

		void SetNumCells(uint32_t numCells);
		void SetCellSize(uint32_t cellSize);
		void SetViewDistance(uint32_t viewDistance);

		uint32_t GetNumCells();
		uint32_t GetCellSize();
		uint32_t GetViewDistance();
		uint32_t GetNumBlocks();

	protected:
		std::map<BlockKey, SharedPtr<Block2>> mBlockList;
		Vk::Renderer* mRenderer;
		int32_t mCellsInBlock = 32;
		int32_t mCellSize = 40;
		int32_t mViewDistance = 4;
	};
}
