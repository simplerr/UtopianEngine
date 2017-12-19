#pragma once

#include <string>
#include <map>
#include <vulkan/vulkan.h>
#include "../external/assimp/assimp/Importer.hpp"
#include "vulkan/Vertex.h"

namespace Vulkan
{
	class StaticModel;
	class TextureLoader;
	class Device;

	struct AssimpMesh
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::string texturePath;
	};

	// TODO: This will later work like a factory, where the same model only gets loaded once
	class ModelLoader
	{
	public:
		ModelLoader(TextureLoader* textureLoader);
		void CleanupModels(VkDevice device);

		StaticModel* LoadModel(Device* device, std::string filename);		// NOTE: TODO: Not a good idea to take VulkanBase as argument
		StaticModel* GenerateTerrain(Device* device, std::string filename);
		StaticModel* LoadDebugBox(Device* device);
		StaticModel* LoadQuad(Device* device);
		StaticModel* ModelLoader::LoadGrid(Device* device, float cellSize, int numCells);
	private:
		int FindValidPath(aiString* texturePath, std::string modelPath);
		bool TryLongerPath(char* szTemp, aiString* p_szString);
		std::map<std::string, StaticModel*> mModelMap;

		TextureLoader* mTextureLoader;
	};
}	// VulkanLib namespace
