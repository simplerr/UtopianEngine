#pragma once

#include <string>
#include <map>
#include "vulkan/VulkanPrerequisites.h"
#include "../external/assimp/assimp/Importer.hpp"
#include "../external/assimp/assimp/material.h"
#include "vulkan/Vertex.h"
#include "utility/Module.h"
#include "utility/Common.h"

struct aiMaterial;

#define PLACEHOLDER_MODEL_PATH "data/models/teapot.obj"
#define PLACEHOLDER_TEXTURE_PATH "data/textures/prototype/Light/texture_12.png"

namespace Utopian
{
   class Model;

   /**
    * Loader supporting a lot of different file formats.
    * E.g .fbx, .obj, .dae, .mdl, .md2, .md3 etc.
    *
    * @note Does not load PBR materials, use .gltf models for that.
    */
   class AssimpLoader
   {
   public:
      AssimpLoader(Vk::Device* device);
      ~AssimpLoader();

      SharedPtr<Model> LoadModel(std::string filename);

   private:
      std::string GetPath(aiMaterial* material, aiTextureType textureType, std::string filename);
      int FindValidPath(aiString* texturePath, std::string modelPath);
      bool TryLongerPath(char* szTemp, aiString* p_szString);

      // Move this, duplicate with glTFLoader
      SharedPtr<Vk::DescriptorSetLayout> mMeshTexturesDescriptorSetLayout;
      SharedPtr<Vk::DescriptorPool> mMeshTexturesDescriptorPool;

      Vk::Device* mDevice;
   };
}