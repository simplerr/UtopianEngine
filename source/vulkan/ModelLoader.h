#pragma once

#include <string>
#include <map>
#include "vulkan/VulkanInclude.h"
#include "../external/assimp/assimp/Importer.hpp"
#include "vulkan/Vertex.h"
#include "utility/Module.h"

namespace Utopian::Vk
{
	struct AssimpMesh
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::string texturePath;
	};

	// TODO: This will later work like a factory, where the same model only gets loaded once
	class ModelLoader : public Module<ModelLoader>
	{
	public:
		ModelLoader(Device* device, TextureLoader* textureLoader);
		void CleanupModels(VkDevice device);

		StaticModel* LoadModel(std::string filename);		// NOTE: TODO: Not a good idea to take VulkanBase as argument
		StaticModel* GenerateTerrain(std::string filename);
		StaticModel* LoadDebugBox();
		StaticModel* LoadQuad();
		StaticModel* LoadGrid(float cellSize, int numCells);
	private:
		int FindValidPath(aiString* texturePath, std::string modelPath);
		bool TryLongerPath(char* szTemp, aiString* p_szString);
		std::map<std::string, StaticModel*> mModelMap;

		TextureLoader* mTextureLoader;
		Device* mDevice;
	};

	ModelLoader& gModelLoader();
}	// VulkanLib namespace
