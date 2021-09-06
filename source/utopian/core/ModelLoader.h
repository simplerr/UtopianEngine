#pragma once

#include <string>
#include <map>
#include "vulkan/VulkanPrerequisites.h"
#include "../external/assimp/assimp/Importer.hpp"
#include "../external/assimp/assimp/material.h"
#include "core/AssimpLoader.h"
#include "vulkan/Vertex.h"
#include "utility/Module.h"
#include "utility/Common.h"

struct aiMaterial;

#define PLACEHOLDER_MODEL_PATH "data/models/teapot.obj"
#define PLACEHOLDER_TEXTURE_PATH "data/textures/prototype/Light/texture_12.png"

namespace Utopian {
   class Model;
   class glTFLoader;
}

namespace Utopian::Vk
{
   // TODO: This will later work like a factory, where the same model only gets loaded once
   class ModelLoader : public Module<ModelLoader>
   {
   public:
      ModelLoader(Device* device);
      ~ModelLoader();

      void CleanupModels(VkDevice device);

      SharedPtr<StaticModel> LoadModel(std::string filename);
      SharedPtr<StaticModel> LoadBox(std::string texture = PLACEHOLDER_TEXTURE_PATH);
      SharedPtr<StaticModel> LoadGrid(float cellSize, int numCells);

      SharedPtr<Model> LoadModel2(std::string filename);
      SharedPtr<Model> LoadBox2(std::string texture = PLACEHOLDER_TEXTURE_PATH);
      SharedPtr<Model> LoadGrid2(float cellSize, int numCells);
      SharedPtr<Model> LoadQuad();

      SharedPtr<DescriptorSetLayout> GetMeshTextureDescriptorSetLayout();
      SharedPtr<DescriptorPool> GetMeshTextureDescriptorPool();
   private:
      std::string GetPath(aiMaterial* material, aiTextureType textureType, std::string filename);
      int FindValidPath(aiString* texturePath, std::string modelPath);
      bool TryLongerPath(char* szTemp, aiString* p_szString);
      std::map<std::string, SharedPtr<StaticModel>> mModelMap;
      std::map<std::string, SharedPtr<Model>> mModelMap2;
      SharedPtr<StaticModel> mPlaceholderModel = nullptr;
      SharedPtr<Model> mPlaceholderModel2 = nullptr;

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

      SharedPtr<AssimpLoader> mAssimpLoader;
      SharedPtr<glTFLoader> mglTFLoader;
   };

   ModelLoader& gModelLoader();
} // VulkanLib namespace