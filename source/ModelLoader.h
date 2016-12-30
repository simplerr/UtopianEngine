#pragma once

#include <string>
#include <map>
#include <vulkan/vulkan.h>

namespace VulkanLib
{
	class StaticModel;
	class VulkanBase;

	// TODO: This will later work like a factory, where the same model only gets loaded once
	class ModelLoader
	{
	public:
		void CleanupModels(VkDevice device);

		StaticModel* LoadModel(VulkanBase* vulkanBase, std::string filename);		// NOTE: TODO: Not a good idea to take VulkanBase as argument
		StaticModel* GenerateTerrain(VulkanBase* vulkanBase, std::string filename);
	private:
		std::map<std::string, StaticModel*> mModelMap;
	};
}	// VulkanLib namespace
