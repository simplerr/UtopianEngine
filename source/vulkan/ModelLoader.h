#pragma once

#include <string>
#include <map>
#include <vulkan/vulkan.h>
#include "../external/assimp/assimp/Importer.hpp"

namespace VulkanLib
{
	class StaticModel;
	class Device;

	// TODO: This will later work like a factory, where the same model only gets loaded once
	class ModelLoader
	{
	public:
		void CleanupModels(VkDevice device);

		StaticModel* LoadModel(Device* device, std::string filename);		// NOTE: TODO: Not a good idea to take VulkanBase as argument
		StaticModel* GenerateTerrain(Device* device, std::string filename);
	private:
		int FindValidPath(aiString* texturePath, std::string modelPath);
		bool TryLongerPath(char* szTemp, aiString* p_szString);
		std::map<std::string, StaticModel*> mModelMap;
	};
}	// VulkanLib namespace