#include "PerlinTerrain.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/Mesh.h"
#include "vulkan/StaticModel.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/Vertex.h"
#include "vulkan/Texture.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/Debug.h"
#include "core/ScriptExports.h"
#include "Camera.h"
#include "Input.h"
#include <random>

namespace Utopian
{
	PerlinTerrain::PerlinTerrain(Vk::Device* device, Camera* camera)
		: BaseTerrain(device, camera)
	{
		mPerlinNoise = std::make_shared<PerlinNoise<float>>(std::random_device{}());
		GenerateGrassInstances(mCamera->GetPosition());
	}

	float PerlinTerrain::GetHeight(float x, float z)
	{
		// Done in Lua for now

		//float frequency = (1.0f / 33000.0f);
		//float amplitude = 11000.0f;
		//float height = 0.0f;

		//const uint32_t octaves = 8;
		//for (uint32_t i = 0; i < octaves; i++)
		//{
		//	height += mPerlinNoise->Noise(x * frequency, 0, z * frequency) * amplitude;
		//	amplitude *= 0.5f;
		//	frequency *= 2;
		//}

		float height = ScriptImports::GetTerrainHeight(x, z);
		return height;
	}

	glm::vec3 PerlinTerrain::GetNormal(float x, float z)
	{
		// Calculate normal, based on https://www.gamedev.net/forums/topic/692347-finite-difference-normal-calculation-for-sphere/
		//glm::vec3 pos = glm::vec3(posX, height, posZ);
		float offset = 1.0f;
		float hL = GetHeight(x - offset, z);
		float hR = GetHeight(x + offset, z);
		float hD = GetHeight(x, z - offset);
		float hU = GetHeight(x, z + offset);

		glm::vec3 normal = glm::vec3(hL - hR, -2.0f, hD - hU); // Note: -2.0f
		normal = normalize(normal);
		
		return normal;
	}

	void PerlinTerrain::Update()
	{
		BaseTerrain::Update();

		float distanceDelta = glm::length(mCamera->GetPosition() - mLastGrassGenPosition);
		if (distanceDelta > 500.0f || gInput().KeyPressed('G'))
		{
			GenerateGrassInstances(mCamera->GetPosition());
		}
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
		Vk::Mesh* mesh = new Vk::Mesh(mDevice);

		float ANY = 0;
		for (int x = 0; x < mCellsInBlock; x++)
		{
			for (int z = 0; z < mCellsInBlock; z++)
			{
				glm::vec3 worldPos = glm::vec3(blockPosition.x - x * mCellSize, 0.0f, blockPosition.z - z * mCellSize);
				worldPos.y = GetHeight(worldPos.x, worldPos.z);
				glm::vec3 normal = GetNormal(worldPos.x, worldPos.z);
				glm::vec3 localPos = glm::vec3(x * mCellSize, 0.0f, z * mCellSize);
				glm::vec2 texcord = glm::vec2(localPos.x / (mCellSize * (mCellsInBlock - 1)), localPos.z / (mCellSize * (mCellsInBlock - 1)));

				// Todo: Calculate tangent and bitangent
				mesh->AddVertex(Vk::Vertex(localPos.x, worldPos.y, localPos.z,
										   normal.x, normal.y, normal.z,
										   ANY, ANY, ANY,
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

		// Calculate tangents and bitangents
		// References:
		// https://learnopengl.com/Advanced-Lighting/Normal-Mapping
		// https://www.youtube.com/watch?v=0QhR7WSoF78
		for (int i = 0; i < mesh->indexVector.size(); i += 3)
		{
			Vk::Vertex v0 = mesh->vertexVector[mesh->indexVector[i]];
			Vk::Vertex v1 = mesh->vertexVector[mesh->indexVector[i+1]];
			Vk::Vertex v2 = mesh->vertexVector[mesh->indexVector[i+2]];

			glm::vec3 e1 = v1.Pos - v0.Pos;
			glm::vec3 e2 = v2.Pos - v0.Pos;

			glm::vec2 deltaUV1 = v1.Tex - v0.Tex;
			glm::vec2 deltaUV2 = v2.Tex - v0.Tex;

			float r = (float)(1.0 / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x));

			glm::vec3 tangent = r * (e1 * deltaUV2.y - e2 * deltaUV1.y);
			glm::vec3 bitangent = r * (e2 * deltaUV1.x - e1 * deltaUV2.x);

			tangent = normalize(tangent);
			bitangent = normalize(bitangent);

			// Tangents and bitangents are shared between all vertices in a triangle
			mesh->vertexVector[mesh->indexVector[i]].Tangent = tangent;
			mesh->vertexVector[mesh->indexVector[i+1]].Tangent = tangent;
			mesh->vertexVector[mesh->indexVector[i+2]].Tangent = tangent;

			mesh->vertexVector[mesh->indexVector[i]].Bitangent = bitangent;
			mesh->vertexVector[mesh->indexVector[i+1]].Bitangent = bitangent;
			mesh->vertexVector[mesh->indexVector[i+2]].Bitangent = bitangent;
		}

		/*mesh->SetTexturePath("data/NatureManufacture/Meadow Environment Dynamic Nature/Ground/T_ground_meadow_grass_01_A_SM.tga");
		Vk::Texture* texture = Vk::gTextureLoader().LoadTexture("data/NatureManufacture/Meadow Environment Dynamic Nature/Ground/T_forest_ground_grass_01_A_SM.tga");
		Vk::Texture* texture = Vk::gTextureLoader().LoadTexture("data/NatureManufacture/Meadow Environment Dynamic Nature/Ground/T_ground_meadow_grass_01_A_SM.tga");*/
		mesh->LoadTextures("data/textures/ground/grass2.tga");
		//mesh->LoadTextures("data/NatureManufacture/Meadow Environment Dynamic Nature/Ground/T_forest_ground_grass_01_A_SM.tga", "data/NatureManufacture/Meadow Environment Dynamic Nature/Ground/T_forest_ground_grass_01_N.png");
		mesh->BuildBuffers(mDevice);
		model->AddMesh(mesh);

		// Generate block renderable
		block->renderable = std::make_shared<Renderable>();
		block->renderable->SetModel(model);
		block->renderable->Initialize();
		block->renderable->SetPosition(blockPosition);
		block->renderable->SetColor(glm::vec4(color, 1.0f));
		block->renderable->SetTileFactor(glm::vec2(10.0f));
		//block->renderable->AppendRenderFlags(RenderFlags::RENDER_FLAG_TERRAIN);
		https://forum.unity.com/threads/foliage-similar-to-the-witness.399585/

		//block->GenerateGrassInstances(mVulkanApp, this, mCellsInBlock, mCellSize);

		mBlockList[blockKey] = block;
	}

	void PerlinTerrain::GenerateBlocks()
	{

	}

	void PerlinTerrain::GenerateGrassInstances(glm::vec3 origin)
	{
		// Todo: Remove
		return;
		mLastGrassGenPosition = origin;

		std::default_random_engine rndGenerator((unsigned)time(nullptr));
		std::uniform_real_distribution<float> uniformDist(-2 * mCellsInBlock * mCellSize, 2 * mCellsInBlock * mCellSize);

		if (mInstanceBuffer != nullptr)
			mGrassInstances.clear();

		const uint32_t numGrassInstances = 32000;
		for (uint32_t i = 0; i < numGrassInstances; i++)
		{
			GrassInstance grassInstance;
			grassInstance.position = glm::vec4(origin.x + uniformDist(rndGenerator), 0.0f, origin.z + uniformDist(rndGenerator), 1.0f);
			grassInstance.position.y = GetHeight(grassInstance.position.x, grassInstance.position.z);
			mGrassInstances.push_back(grassInstance);
		}

		if (mInstanceBuffer == nullptr)
		{
			// Todo: use device local buffer for better performance
			mInstanceBuffer = std::make_shared<Vk::Buffer>(mDevice,
														   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
														   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
														   mGrassInstances.size() * sizeof(GrassInstance),
														   mGrassInstances.data());
		}
		else
		{
			mInstanceBuffer->UpdateMemory(mGrassInstances.data(),
										  mGrassInstances.size() * sizeof(GrassInstance));
		}
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
