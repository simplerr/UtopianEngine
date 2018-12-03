#include "core/terrain/BaseTerrain.h"
#include "vulkan/Renderer.h"
#include "Camera.h"

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
		int32_t blockX = cameraPos.x / (float)(mVoxelSize * mVoxelsInBlock) + 1;
		int32_t blockY = cameraPos.y / (float)(mVoxelSize * mVoxelsInBlock) + 1;
		int32_t blockZ = cameraPos.z / (float)(mVoxelSize * mVoxelsInBlock) + 1;

		// Make all blocks invisible
		for (auto blockIter : mBlockList)
		{
			blockIter.second->visible = false;
		}

		for (int32_t x = blockX - mViewDistance; x <= (blockX + mViewDistance); x++)
		{
			for (int32_t z = blockZ - mViewDistance; z <= (blockZ + mViewDistance); z++)
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

		// Some terrains will need to generate the block vertex buffers
		GenerateBlocks();
	}

	std::map<BlockKey, SharedPtr<Block2>>& BaseTerrain::GetBlocks()
	{
		return mBlockList;
	}
}