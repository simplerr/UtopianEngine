#include "core/terrain/BaseTerrain.h"
#include "core/renderer/RenderingManager.h"
#include "vulkan/Renderer.h"
#include "Camera.h"
#include <random>

namespace Utopian
{
	bool operator<(BlockKey const& a, BlockKey const& b)
	{
		if (a.x != b.x)
			return (a.x < b.x);

		if (a.y != b.y)
			return (a.y < b.y);

		return (a.z < b.z);
	}

	Block2::~Block2()
	{
		renderable->OnDestroyed();
	}

	void Block2::GenerateGrassInstances(Vk::Renderer* renderer, BaseTerrain* terrain, int32_t cellsInBlock, int32_t cellSize)
	{
		std::default_random_engine rndGenerator((unsigned)time(nullptr));
		std::uniform_real_distribution<float> uniformDist(-cellsInBlock * cellSize, 0);

		if (instanceBuffer != nullptr)
			grassInstances.clear();

		const uint32_t numGrassInstances = 1000;
		for (uint32_t i = 0; i < numGrassInstances; i++)
		{
			GrassInstance grassInstance;
			grassInstance.position = glm::vec4(position.x + uniformDist(rndGenerator), 0.0f, position.z + uniformDist(rndGenerator), 1.0f);
			grassInstance.position.y = terrain->GetHeight(grassInstance.position.x, grassInstance.position.z);
			grassInstance.textureIndex = (rand() % 100) < 97 ? 0 : 1;
			grassInstance.color = color;

			// Only add grass on flat surface and low altitudes
			// Note: Negative sign
			if (terrain->GetNormal(grassInstance.position.x, grassInstance.position.z).y < -0.8f && grassInstance.position.y > -800.0f)
			{
				grassInstances.push_back(grassInstance);
				grassGenerated = true;
			}
			else
			{
				volatile int a = 1;
			}
		}

		if (grassGenerated && instanceBuffer == nullptr)
		{
			// Todo: use device local buffer for better performance
			instanceBuffer = std::make_shared<Vk::Buffer>(renderer->GetDevice(),
														   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
														   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
														   grassInstances.size() * sizeof(GrassInstance),
														   grassInstances.data());
		}
		else if(grassGenerated)
		{
			instanceBuffer->UpdateMemory(grassInstances.data(),
										  grassInstances.size() * sizeof(GrassInstance));
		}
		else
		{
			hasGrass = false;
		}

	}

	BaseTerrain::BaseTerrain(Vk::Renderer* renderer)
	{
		mRenderer = renderer;
	}

	void BaseTerrain::Update()
	{
		// 1) Which blocks should be rendered? Based on the camera position
		// Transform the camera position in to block grid coordinate
		// 2) Are they already added? 
		// 3) Add them

		glm::vec3 cameraPos = mRenderer->GetCamera()->GetPosition();
		int32_t blockX = cameraPos.x / (float)(mCellSize * mCellsInBlock) + 1;
		int32_t blockY = cameraPos.y / (float)(mCellSize * mCellsInBlock) + 1;
		int32_t blockZ = cameraPos.z / (float)(mCellSize * mCellsInBlock) + 1;

		// Make all blocks and their grass invisible
		for (auto blockIter : mBlockList)
		{
			blockIter.second->visible = false;
			blockIter.second->grassVisible = false;
		}

		// Add blocks within block view distance
		int32_t blockViewDistance = gRenderingManager().GetRenderingSettings().blockViewDistance;
		for (int32_t x = blockX - blockViewDistance; x <= (blockX + blockViewDistance); x++)
		{
			for (int32_t z = blockZ - blockViewDistance; z <= (blockZ + blockViewDistance); z++)
			{
				//for (int32_t y = blockY - mViewDistance; y <= (blockY + mViewDistance); y++)
				{
					BlockKey blockKey(x, 0, z);
					if (mBlockList.find(blockKey) == mBlockList.end())
					{
						AddBlock(blockKey);
						//Vulkan::VulkanDebug::ConsolePrint(x, "loaded blockX: ");
						//Vulkan::VulkanDebug::ConsolePrint(z, "loaded blockZ: ");
					}
					else
					{
						mBlockList[blockKey]->visible = true;
					}
				}
			}
		}

		// Generate grass instance buffers for blocks within grass view distance
		// Todo: Note: There is something off in this calculation when moving in negative coordinates and large ones
		float grassViewDistanceSetting = gRenderingManager().GetRenderingSettings().grassViewDistance;
		int32_t grassViewDistance = (grassViewDistanceSetting + (mCellSize * mCellsInBlock)) / (mCellSize * mCellsInBlock);
		for (int32_t x = blockX - grassViewDistance; x <= (blockX + grassViewDistance); x++)
		{
			for (int32_t z = blockZ - grassViewDistance; z <= (blockZ + grassViewDistance); z++)
			{
				BlockKey blockKey(x, 0, z);
				if (mBlockList.find(blockKey) != mBlockList.end())
				{
					if (!mBlockList[blockKey]->grassGenerated && mBlockList[blockKey]->hasGrass)
						mBlockList[blockKey]->GenerateGrassInstances(mRenderer, this, mCellsInBlock, mCellSize);

					mBlockList[blockKey]->grassVisible = true;
				}
				else
				{
					assert(0); // Should always be a block generated before generating grass
				}
			}
		}

		// Some terrains will need to generate the block vertex buffers
		GenerateBlocks();
	}

	std::map<BlockKey, SharedPtr<Block2>>& BaseTerrain::GetBlocks()
	{
		return mBlockList;
	}
	
	void BaseTerrain::ClearBlocks()
	{
		mBlockList.clear();
	}

	void BaseTerrain::SetNumCells(uint32_t numCells)
	{
		mCellsInBlock = numCells;
	}

	void BaseTerrain::SetCellSize(uint32_t cellSize)
	{
		mCellSize = cellSize;
	}

	uint32_t BaseTerrain::GetNumCells()
	{
		return mCellsInBlock;
	}

	uint32_t BaseTerrain::GetCellSize()
	{
		return mCellSize;
	}

	uint32_t BaseTerrain::GetNumBlocks()
	{
		return mBlockList.size();
	}
}