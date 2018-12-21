#include "PerlinTerrain.h"
#include "vulkan/Renderer.h"
#include "vulkan/Mesh.h"
#include "vulkan/StaticModel.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/Vertex.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/TextureLoader.h"
#include <random>

namespace Utopian
{
	PerlinTerrain::PerlinTerrain(Vk::Renderer* renderer)
		: BaseTerrain(renderer)
	{
		GenerateGrassInstances();
	}

	float PerlinTerrain::GetHeight(float x, float z)
	{
		const float frequency = 2200.0f;
		const float amplitude = 550.0f;

		// float height = glm::sin(x / frequency) * amplitude;
		// height += glm::sin(z / frequency) * amplitude;

		float height = mPerlinNoise.noise(x / frequency, 0, z / frequency) * amplitude;
		height += mPerlinNoise.noise(x / frequency / 10.0f, 0, z / frequency / 10.0f) * amplitude * 3;

		return height;
	}

	void PerlinTerrain::AddBlock(BlockKey blockKey)
	{
		glm::vec3 blockPosition;
		blockPosition.x = blockKey.x * (mCellsInBlock - 1) * mCellSize;
		blockPosition.y = blockKey.y * mCellsInBlock * mCellSize;
		blockPosition.z = blockKey.z * (mCellsInBlock - 1) * mCellSize;
		glm::vec3 color = glm::vec3((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f);
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		SharedPtr<Block2> block = std::make_shared<Block2>(blockPosition, color);

		Vk::StaticModel* model = new Vk::StaticModel();
		Vk::Mesh* mesh = new Vk::Mesh(mRenderer->GetDevice());

		float ANY = 0;
		for (int x = 0; x < mCellsInBlock; x++)
		{
			for (int z = 0; z < mCellsInBlock; z++)
			{
				glm::vec3 worldPos = glm::vec3(blockPosition.x - x * mCellSize, 0.0f, blockPosition.z - z * mCellSize);
				worldPos.y = GetHeight(worldPos.x, worldPos.z);
				glm::vec3 localPos = glm::vec3(x * mCellSize, 0.0f, z * mCellSize);
				glm::vec2 texcord = glm::vec2(localPos.x / (mCellSize * (mCellsInBlock - 1)), localPos.z / (mCellSize * (mCellsInBlock - 1)));

				// Calculate normal, based on https://www.gamedev.net/forums/topic/692347-finite-difference-normal-calculation-for-sphere/
				//glm::vec3 pos = glm::vec3(posX, height, posZ);
				float offset = 1.0f;
				float hL = GetHeight(worldPos.x - offset, worldPos.z);
				float hR = GetHeight(worldPos.x + offset, worldPos.z);
				float hD = GetHeight(worldPos.x, worldPos.z - offset);
				float hU = GetHeight(worldPos.x, worldPos.z + offset);

				glm::vec3 normal = glm::vec3(hL - hR, -2.0f, hD - hU); // Note: -2.0f
				normal = glm::normalize(normal);

				mesh->AddVertex(Vk::Vertex(localPos.x, worldPos.y, localPos.z,
										   normal.x, normal.y, normal.z,
										   ANY, ANY, ANY,
										   texcord.x, texcord.y,
										   ANY, ANY, ANY));
			}
		}

		for (int x = 0; x < mCellsInBlock - 1; x++)
		{
			for (int z = 0; z < mCellsInBlock - 1; z++)
			{
				mesh->AddTriangle(x * mCellsInBlock + z, x * mCellsInBlock + z + 1, (x + 1) * mCellsInBlock + z);
				mesh->AddTriangle((x + 1) * mCellsInBlock + z, x * mCellsInBlock + z + 1, (x + 1) * mCellsInBlock + (z + 1));
			}
		}

		mesh->SetTexturePath("data/textures/grass2.png");
		Vk::Texture* texture = Vk::gTextureLoader().LoadTexture("data/textures/grass2.png");
		mesh->SetTexture(texture);
		mesh->BuildBuffers(mRenderer->GetDevice());
		model->AddMesh(mesh);

		// Generate block renderable
		block->renderable = std::make_shared<Renderable>();
		block->renderable->SetModel(model);
		block->renderable->Initialize();
		block->renderable->SetPosition(blockPosition);
		block->renderable->SetColor(glm::vec4(color, 1.0f));
		//block->renderable->AppendRenderFlags(RenderFlags::RENDER_FLAG_NORMAL_DEBUG);

		mBlockList[blockKey] = block;
	}

	void PerlinTerrain::GenerateBlocks()
	{

	}

	void PerlinTerrain::GenerateGrassInstances()
	{
		std::default_random_engine rndGenerator((unsigned)time(nullptr));
		std::uniform_real_distribution<float> uniformDist(0.0, 1 * mCellsInBlock * mCellSize);

		const uint32_t numGrassInstances = 500;
		for (uint32_t i = 0; i < numGrassInstances; i++)
		{
			GrassInstance grassInstance;
			grassInstance.position = glm::vec4(uniformDist(rndGenerator), 0.0f, uniformDist(rndGenerator), 1.0f);
			grassInstance.position.y = GetHeight(grassInstance.position.x, grassInstance.position.z);
			mGrassInstances.push_back(grassInstance);
		}

		// Todo: use device local buffer for better performance
		mInstanceBuffer = std::make_shared<Vk::Buffer>(mRenderer->GetDevice(),
													   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
												       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
												       mGrassInstances.size() * sizeof(GrassInstance),
												       mGrassInstances.data());
	}

	Vk::Buffer* PerlinTerrain::GetInstanceBuffer()
	{
		return mInstanceBuffer.get();
	}

	uint32_t PerlinTerrain::GetNumInstances()
	{
		return mGrassInstances.size();
	}

}
