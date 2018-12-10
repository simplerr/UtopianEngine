#include "PerlinTerrain.h"
#include "vulkan/Renderer.h"
#include "vulkan/Mesh.h"
#include "vulkan/StaticModel.h"
#include "vulkan/Vertex.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/TextureLoader.h"

namespace Utopian
{
	PerlinTerrain::PerlinTerrain(Vk::Renderer* renderer)
		: BaseTerrain(renderer)
	{
	}

	void PerlinTerrain::AddBlock(BlockKey blockKey)
	{
		glm::vec3 blockPosition;
		blockPosition.x = blockKey.x * (mVoxelsInBlock - 1) * mVoxelSize;
		blockPosition.y = blockKey.y * mVoxelsInBlock * mVoxelSize;
		blockPosition.z = blockKey.z * (mVoxelsInBlock - 1) * mVoxelSize;
		glm::vec3 color = glm::vec3((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f);
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		SharedPtr<Block2> block = std::make_shared<Block2>(blockPosition, color);

		Vk::StaticModel* model = new Vk::StaticModel();
		Vk::Mesh* mesh = new Vk::Mesh(mRenderer->GetDevice());

		float ANY = 0;
		for (int x = 0; x < mVoxelsInBlock; x++)
		{
			for (int z = 0; z < mVoxelsInBlock; z++)
			{
				const float frequency = 500.0f;
				const float amplitude = 250.0f;
				float height = glm::sin((-x * mVoxelSize + blockPosition.x) / frequency) * amplitude;
				height += glm::sin((-z * mVoxelSize + blockPosition.z) / frequency) * amplitude;
				const float originOffset = (mVoxelSize * mVoxelsInBlock) / 2.0f;
				glm::vec3 originInCenterPos = glm::vec3(x * mVoxelSize - originOffset, height, z * mVoxelSize - originOffset);
				glm::vec3 voxelPos = glm::vec3(x * mVoxelSize, 0.0f, z * mVoxelSize);
				glm::vec2 texcord = glm::vec2(voxelPos.x / (mVoxelSize * (mVoxelsInBlock - 1)), voxelPos.z / (mVoxelSize * (mVoxelsInBlock - 1)));

				// Note: normal.y is -1 when it's expected to be 1
				mesh->AddVertex(Vk::Vertex(originInCenterPos.x, originInCenterPos.y, originInCenterPos.z, 0.0f, -1.0f, 0.0f, ANY, ANY, ANY, texcord.x, texcord.y, ANY, ANY, ANY));
			}
		}

		for (int x = 0; x < mVoxelsInBlock - 1; x++)
		{
			for (int z = 0; z < mVoxelsInBlock - 1; z++)
			{
				mesh->AddTriangle(x * mVoxelsInBlock + z, x * mVoxelsInBlock + z + 1, (x + 1) * mVoxelsInBlock + z);
				mesh->AddTriangle((x + 1) * mVoxelsInBlock + z, x * mVoxelsInBlock + z + 1, (x + 1) * mVoxelsInBlock + (z + 1));
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
		block->renderable->AppendRenderFlags(RenderFlags::RENDER_FLAG_NORMAL_DEBUG);

		mBlockList[blockKey] = block;
	}

	void PerlinTerrain::GenerateBlocks()
	{

	}
}
