#include <fstream>
#include <random>
#include <numeric>
#include "vulkan/Renderer.h"
#include "vulkan/VulkanDebug.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/ShaderManager.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/Pipeline.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/handles/Queue.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/handles/CommandBuffer.h"
#include "Camera.h"
#include "Terrain.h"
#include "Block.h"
#include "MarchingCubes.h"

bool operator<(BlockKey const& a, BlockKey const& b)
{
	if (a.x != b.x)
       return (a.x < b.x);

   if (a.y != b.y)
       return (a.y < b.y);

   return (a.z < b.z);
}

// Translation of Ken Perlin's JAVA implementation (http://mrl.nyu.edu/~perlin/noise/)
template <typename T>
class PerlinNoise
{
private:
	uint32_t permutations[512];
	T fade(T t)
	{
		return t * t * t * (t * (t * (T)6 - (T)15) + (T)10);
	}
	T lerp(T t, T a, T b)
	{
		return a + t * (b - a);
	}
	T grad(int hash, T x, T y, T z)
	{
		// Convert LO 4 bits of hash code into 12 gradient directions
		int h = hash & 15;
		T u = h < 8 ? x : y;
		T v = h < 4 ? y : h == 12 || h == 14 ? x : z;
		return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
	}
public:
	PerlinNoise()
	{
		// Generate random lookup for permutations containing all numbers from 0..255
		std::vector<uint8_t> plookup;
		plookup.resize(256);
		std::iota(plookup.begin(), plookup.end(), 0);
		std::default_random_engine rndEngine(std::random_device{}());
		std::shuffle(plookup.begin(), plookup.end(), rndEngine);

		for (uint32_t i = 0; i < 256; i++)
		{
			permutations[i] = permutations[256 + i] = plookup[i];
		}
	}
	T noise(T x, T y, T z)
	{
		// Find unit cube that contains point
		int32_t X = (int32_t)floor(x) & 255;
		int32_t Y = (int32_t)floor(y) & 255;
		int32_t Z = (int32_t)floor(z) & 255;
		// Find relative x,y,z of point in cube
		x -= floor(x);
		y -= floor(y);
		z -= floor(z);

		// Compute fade curves for each of x,y,z
		T u = fade(x);
		T v = fade(y);
		T w = fade(z);

		// Hash coordinates of the 8 cube corners
		uint32_t A = permutations[X] + Y;
		uint32_t AA = permutations[A] + Z;
		uint32_t AB = permutations[A + 1] + Z;
		uint32_t B = permutations[X + 1] + Y;
		uint32_t BA = permutations[B] + Z;
		uint32_t BB = permutations[B + 1] + Z;

		// And add blended results for 8 corners of the cube;
		T res = lerp(w, lerp(v,
			lerp(u, grad(permutations[AA], x, y, z), grad(permutations[BA], x - 1, y, z)), lerp(u, grad(permutations[AB], x, y - 1, z), grad(permutations[BB], x - 1, y - 1, z))),
			lerp(v, lerp(u, grad(permutations[AA + 1], x, y, z - 1), grad(permutations[BA + 1], x - 1, y, z - 1)), lerp(u, grad(permutations[AB + 1], x, y - 1, z - 1), grad(permutations[BB + 1], x - 1, y - 1, z - 1))));
		return res;
	}
};

void GenerateNoiseTexture(float texture3d[], int width, int height, int depth)
{
	memset(texture3d, 0x00, sizeof(float) * width * height * depth);

	PerlinNoise<float> perlinNoise;

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			for (int z = 0; z < depth; z++)
			{
				float n = perlinNoise.noise((float)x / width, (float)y / height, (float)z / depth);
				texture3d[x + y * width + z * width * height] = n;
			}
		}
	}
}

Terrain::Terrain(Utopian::Vk::Renderer* renderer)
{
	mRenderer = renderer;

	/* 
		Initialize Vulkan handles
	*/

	// NOTE: This must be done before Init()
	// Move to effect?
	mMarchingCubesEffect.edgeTableTex = mRenderer->mTextureLoader->CreateTexture(edgeTable, VK_FORMAT_R32_UINT, 256, 1, 1, sizeof(int));
	mMarchingCubesEffect.triangleTableTex = mRenderer->mTextureLoader->CreateTexture(triTable, VK_FORMAT_R32_UINT, 16, 256, 1, sizeof(int));

	/* Experimentation */
	const uint32_t w = 32;
	const uint32_t h = 32;
	const uint32_t d = 32;

	float texture3d[w * h * d];
	GenerateNoiseTexture(texture3d, w, h, d);

	mMarchingCubesEffect.texture3d = mRenderer->mTextureLoader->CreateTexture(texture3d, VK_FORMAT_R32_SFLOAT, w, h, d, sizeof(float));

	mTerrainEffect.Init(renderer);
	mMarchingCubesEffect.Init(renderer);

	mMarchingCubesEffect.ubo.data.offsets[0] = vec4(0, 0, 0, 0);
	mMarchingCubesEffect.ubo.data.offsets[1] = vec4(mVoxelSize, 0, 0, 0);
	mMarchingCubesEffect.ubo.data.offsets[2] = vec4(mVoxelSize, mVoxelSize, 0, 0);
	mMarchingCubesEffect.ubo.data.offsets[3] = vec4(0, mVoxelSize, 0, 0);
	mMarchingCubesEffect.ubo.data.offsets[4] = vec4(0, 0, mVoxelSize, 0);
	mMarchingCubesEffect.ubo.data.offsets[5] = vec4(mVoxelSize, 0, mVoxelSize, 0);
	mMarchingCubesEffect.ubo.data.offsets[6] = vec4(mVoxelSize, mVoxelSize, mVoxelSize, 0);
	mMarchingCubesEffect.ubo.data.offsets[7] = vec4(0, mVoxelSize, mVoxelSize, 0);
	mMarchingCubesEffect.ubo.data.color = vec4(0, 1, 0, 1);
	mMarchingCubesEffect.ubo.data.voxelSize = mVoxelSize;
	mMarchingCubesEffect.UpdateMemory();

	mClippingPlane = glm::vec4(0, 1, 0, 1500000);
}

Terrain::~Terrain()
{

}

void Terrain::UpdateBlockList()
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
		blockIter.second->SetVisible(false);
	}

	for (int32_t x = blockX - mViewDistance; x <= (blockX + mViewDistance); x++)
	{
		for (int32_t z = blockZ - mViewDistance; z <= (blockZ + mViewDistance); z++)
		{
			for (int32_t y = blockY - mViewDistance; y <= (blockY + mViewDistance); y++)
			{
				BlockKey blockKey(x, y, z);
				if (mBlockList.find(blockKey) == mBlockList.end())
				{
					glm::vec3 position = glm::vec3(x*mVoxelsInBlock*mVoxelSize, y*mVoxelsInBlock*mVoxelSize, z*mVoxelsInBlock*mVoxelSize);
					glm::vec3 color = glm::vec3((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f);
					//color = glm::vec3(0.0f, 0.7f, 0.0f);
					Utopian::Vk::DescriptorSetLayout* setLayout1 = mMarchingCubesEffect.GetDescriptorSetLayout(SET_1);
					Block* block = new Block(mRenderer, position, color, mVoxelsInBlock, mVoxelSize, setLayout1, mMarchingCubesEffect.GetDescriptorPool()); // NOTE: The descriptor set layout

					mBlockList[blockKey] = block;

					//Vulkan::VulkanDebug::ConsolePrint(x, "loaded blockX: ");
					//Vulkan::VulkanDebug::ConsolePrint(z, "loaded blockZ: ");
				}
				else
				{
					mBlockList[blockKey]->SetVisible(true);
				}
			}
		}
	}
}

void Terrain::GenerateBlocks(float time)
{
	for (auto blockIter : mBlockList)
	{
		Block* block = blockIter.second;
		// Generate the vertex buffer for the block 
		if (!block->IsGenerated() || block->IsModified())
		{
			/* Experimentation */
			const uint32_t w = 16;
			const uint32_t h = 16;
			const uint32_t d = 16;

			mMarchingCubesEffect.ubo.data.projection = mRenderer->GetCamera()->GetProjection();
			mMarchingCubesEffect.ubo.data.view = mRenderer->GetCamera()->GetView();
			mMarchingCubesEffect.ubo.data.voxelSize = mVoxelSize;
			mMarchingCubesEffect.ubo.data.time = time;

			// Reset block vertex count
			mMarchingCubesEffect.mCounterSSBO.numVertices = 0;
			mMarchingCubesEffect.UpdateMemory();

			// Bind new vertex buffer SSBO descriptor
			mMarchingCubesEffect.mDescriptorSet1->BindStorageBuffer(BINDING_0, &block->GetBufferInfo());
			mMarchingCubesEffect.mDescriptorSet1->UpdateDescriptorSets();

			Utopian::Vk::CommandBuffer commandBuffer = Utopian::Vk::CommandBuffer(mRenderer->GetDevice(), mRenderer->GetCommandPool(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

			commandBuffer.CmdBindPipeline(mMarchingCubesEffect.GetComputePipeline());
			// TODO: Use the firstSet parameter to only update the Block descriptor
			VkDescriptorSet descriptorSets[2] = { mMarchingCubesEffect.GetDescriptorSet0(),
												  mMarchingCubesEffect.GetDescriptorSet1() };
			commandBuffer.CmdBindDescriptorSet(&mMarchingCubesEffect, 2, descriptorSets, VK_PIPELINE_BIND_POINT_COMPUTE);

			// Push the world matrix constant
			Utopian::Vk::PushConstantBlock pushConsts(glm::translate(glm::mat4(), block->GetPosition()));

			commandBuffer.CmdPushConstants(&mMarchingCubesEffect, VK_SHADER_STAGE_COMPUTE_BIT, sizeof(pushConsts), &pushConsts);
			commandBuffer.CmdDispatch(32, 32, 32);
			commandBuffer.Flush(mRenderer->GetQueue()->GetVkHandle(), mRenderer->GetCommandPool());

			// Get # of vertices so we can tell how many to draw
			uint32_t* mapped;
			mMarchingCubesEffect.mCounterSSBO.MapMemory(0, sizeof(uint32_t), 0, (void**)&mapped);
			uint32_t numVertices = *(uint32_t*)mapped;
			mMarchingCubesEffect.mCounterSSBO.UnmapMemory();

			block->SetNumVertices(numVertices);
			block->SetGenerated(true);
			block->SetModified(false);
		}
	}
}

void Terrain::Render(Utopian::Vk::CommandBuffer* commandBuffer, Utopian::Vk::DescriptorSet* commonDescriptorSet)
{
	if (mEnabled)
	{
		for (auto blockIter : mBlockList)
		{
			Block* block = blockIter.second;
			if (block->IsVisible() && block->IsGenerated())
			{
				mTerrainEffect.SetPipeline(block->pipelineType);

				commandBuffer->CmdBindPipeline(mTerrainEffect.GetPipeline(0));
				VkDescriptorSet descriptorSets[1] = { commonDescriptorSet->descriptorSet };
				commandBuffer->CmdBindDescriptorSet(&mTerrainEffect, 1, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);

				commandBuffer->CmdBindVertexBuffer(BINDING_0, 1, block->GetVertexBuffer());

				// Push the world matrix constant
				Utopian::Vk::PushConstantBasicBlock pushConstantBlock;
				pushConstantBlock.world = glm::mat4();
				pushConstantBlock.color = block->GetColor();

				pushConstantBlock.world = glm::translate(glm::mat4(), block->GetPosition());
				pushConstantBlock.world[3][0] = -pushConstantBlock.world[3][0];
				pushConstantBlock.world[3][1] = -pushConstantBlock.world[3][1];
				pushConstantBlock.world[3][2] = -pushConstantBlock.world[3][2];

				commandBuffer->CmdPushConstants(&mTerrainEffect, VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConstantBlock), &pushConstantBlock);
				commandBuffer->CmdDraw(block->GetNumVertices(), 1, 0, 0);
			}
		}
	}
}

void Terrain::Update()
{
	if (mEnabled)
	{
		static float time = 0.0f;

		if (mUpdateTimer)
			time += 0.002f;

		UpdateBlockList();
		GenerateBlocks(time);
		UpdateUniformBuffer();

		/*float x = mCamera->GetPosition().x;
		float z = mCamera->GetPosition().z;
		mCamera->SetPosition(glm::vec3(x, GetHeight(x, z) + 100, z));*/
	}
}

void Terrain::UpdateUniformBuffer()
{
	mTerrainEffect.UpdateMemory();
}

void Terrain::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_KEYDOWN:
			if (wParam == 'G')
			{
				uint32_t numBlocks = 0;
				uint32_t numVertices = 0;
				for (auto blockIter : mBlockList)
				{
					Block* block = blockIter.second;
					numBlocks++;
					numVertices += block->GetNumVertices();
				}

				Utopian::Vk::VulkanDebug::ConsolePrint(numBlocks, "numBlocks: ");
				Utopian::Vk::VulkanDebug::ConsolePrint(numVertices, "numVertices: ");

				glm::vec3 cameraPos = mRenderer->GetCamera()->GetPosition();
				int32_t blockX = cameraPos.x / (float)(mVoxelSize * mVoxelsInBlock);
				int32_t blockY = 0;
				int32_t blockZ = cameraPos.z / (float)(mVoxelSize * mVoxelsInBlock);

				Utopian::Vk::VulkanDebug::ConsolePrint(blockX, "blockX: ");
				Utopian::Vk::VulkanDebug::ConsolePrint(blockZ, "blockZ: ");
				Utopian::Vk::VulkanDebug::ConsolePrint(GetHeight(mRenderer->GetCamera()->GetPosition().x, mRenderer->GetCamera()->GetPosition().z), "terrain height: ");

				UpdateBlockList();
			}
		
			break;
	}
}

void Terrain::DumpDebug()
{
	
}

void Terrain::SetClippingPlane(glm::vec4 clippingPlane)
{
	mClippingPlane = clippingPlane;
}

float cosNoise(vec2 pos)
{
	float amplitude = 1.0f;
	float freq = 5500.0f;
	return amplitude * (sin(pos.x/freq) + sin(pos.y / freq));
}
const mat2 m2 = mat2(1.6,-1.2,
                     1.2, 1.6);

float Terrain::Density(glm::vec3 position)
{
	float density = 1;	

	// Magic from Shadertoy: https://www.shadertoy.com/view/MtsSRf
	float h = 0.0;
	vec2 q = glm::vec2(position.x * 0.5, position.z*0.5);

    float s = 0.5;
    for(int i=0; i<1; i++)
    {
        h -= s*cosNoise(q); 
		q = m2*q;// *0.85;
		q *= 0.85;
        q += vec2(2.41,8.13);
        s *= 0.48 + 0.2*h;
    }
    h *= 10500.0;

	density = glm::min(-position.y + h, density);

	return density;
}

float Terrain::GetHeight(float x, float z)
{
	glm::vec3 origin = glm::vec3(x, 25000, z);
	glm::vec3 dir = glm::vec3(0, -1, 0);

	glm::vec3 intersection = GetRayIntersection(origin, dir);

	return intersection.y;
}

glm::vec3 Terrain::GetRayIntersection(glm::vec3 origin, glm::vec3 direction)
{
	glm::vec3 intersection = glm::vec3();
	float tmax = 100000.0;
	float t = 0.0;
	for (uint32_t i = 0; i < 200; i++)
	{
		glm::vec3 pos = origin + direction * t;
		float h = Density(pos);

		if (h > 10.011 || t > tmax)
			break;

		t += -h * 0.5;
	}

	if (t < tmax)
	{
		intersection = (origin + direction * t);
	}

	return intersection;
}

void Terrain::SetEnabled(bool enabled)
{
	mEnabled = enabled;
}
