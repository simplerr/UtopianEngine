#pragma once

#include <string>
#include <map>
#include "vulkan/VulkanInclude.h"
#include "../external/assimp/assimp/Importer.hpp"
#include "vulkan/Vertex.h"
#include "utility/Module.h"
#include "utility/Common.h"

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
		ModelLoader(Device* device);
		void CleanupModels(VkDevice device);

		StaticModel* LoadModel(std::string filename);		// NOTE: TODO: Not a good idea to take VulkanBase as argument
		StaticModel* GenerateTerrain(std::string filename);
		StaticModel* LoadDebugBoxLines();		// Use with VK_PRIMITIVE_TOPOLOGY_LINE_LIST
		StaticModel* LoadDebugBoxTriangles();	// Use with VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
		StaticModel* LoadQuad();
		StaticModel* LoadGrid(float cellSize, int numCells);

		SharedPtr<DescriptorSetLayout> GetMeshTextureDescriptorSetLayout();
		SharedPtr<DescriptorPool> GetMeshTextureDescriptorPool();
	private:
		int FindValidPath(aiString* texturePath, std::string modelPath);
		bool TryLongerPath(char* szTemp, aiString* p_szString);
		std::map<std::string, StaticModel*> mModelMap;

		Device* mDevice;
		
		// The descriptor set layout that contains a diffuse and normal combined image sampler
		// It is expected to be used in multiple shaders so instead of relying on the 
		// shader reflection we manually create a layout that every shader
		// that will have per mesh textures MUST use
		// layout (set = 1, binding = 0) uniform sampler2D diffuseSampler;
		// layout (set = 1, binding = 1) uniform sampler2D normalSampler;
		SharedPtr<DescriptorSetLayout> mMeshTexturesDescriptorSetLayout;
		
		// Descriptor pool for the combined image samplers that will be allocated for mesh textures
		SharedPtr<DescriptorPool> mMeshTexturesDescriptorPool;
	};

	ModelLoader& gModelLoader();
}	// VulkanLib namespace
