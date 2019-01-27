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
		int textureIndex;
		//glm::vec3 scale;
	};

	class BaseTerrain;

	bool operator<(BlockKey const& a, BlockKey const& b);

	class Block2
	{
	public:
		Block2(glm::vec3 pos, glm::vec3 col) {
			position = pos;
			color = col;
			grassGenerated = false;
			grassGenerated = false;
			modified = false;
			visible = false;
			hasGrass = true; // Assume every block will have grass
		}

		~Block2();

		void GenerateGrassInstances(Vk::Device* device, BaseTerrain* terrain, int32_t cellsInBlock, int32_t cellSize);

		SharedPtr<Renderable> renderable;
		std::vector<GrassInstance> grassInstances;
		SharedPtr<Vk::Buffer> instanceBuffer;
		glm::vec3 position;
		glm::vec3 color;
		bool grassGenerated;
		bool grassVisible;
		bool hasGrass;
		bool modified;
		bool visible;
	private:
	};

	class BaseTerrain
	{
	public:
		BaseTerrain(Vk::Device* device, Camera* camera);

		virtual void Update();
		virtual void GenerateBlocks() = 0;
		virtual void AddBlock(BlockKey blockKey) = 0;
		virtual float GetHeight(float x, float z) = 0;
		virtual glm::vec3 GetNormal(float x, float z) = 0;

		std::map<BlockKey, SharedPtr<Block2>>& GetBlocks();
		void ClearBlocks();

		void SetNumCells(uint32_t numCells);
		void SetCellSize(uint32_t cellSize);

		uint32_t GetNumCells();
		uint32_t GetCellSize();
		uint32_t GetNumBlocks();

	protected:
		std::map<BlockKey, SharedPtr<Block2>> mBlockList;
		Vk::Device* mDevice;
		Camera* mCamera;
		int32_t mCellsInBlock = 32;
		int32_t mCellSize = 40;
		// int32_t mBlockViewDistance = 4;
		// float mGrassViewDistance = 2000.0f;
	};
}
