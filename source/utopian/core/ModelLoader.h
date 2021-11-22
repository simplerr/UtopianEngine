#pragma once

#include <string>
#include <map>
#include "vulkan/VulkanPrerequisites.h"
#include "core/renderer/Model.h"
#include "vulkan/Vertex.h"
#include "utility/Module.h"
#include "utility/Common.h"

#define DEFAULT_COLOR_TEXTURE_PATH "data/textures/prototype/Light/texture_12.png"
#define DEFAULT_NORMAL_MAP_TEXTURE "data/textures/flat_normalmap.png"
#define DEFAULT_METALLIC_ROUGHNESS_TEXTURE "data/textures/white_texture.png"
#define DEFAULT_SPECULAR_MAP_TEXTURE "data/textures/black_texture.png"
#define DEFAULT_OCCLUSION_TEXTURE "data/textures/white_texture.png"
#define PLACEHOLDER_MODEL_PATH "data/models/teapot.obj"

namespace Utopian
{
   class Model;
   class AssimpLoader;
   class glTFLoader;

   /**
    * Used for loading models from the filesystem.
    * Also supports creating models with simple shape primitives.
    */
   class ModelLoader : public Module<ModelLoader>
   {
   public:
      ModelLoader(Vk::Device* device);
      ~ModelLoader();

      SharedPtr<Model> LoadModel(std::string filename, bool uniqueInstance = false);
      SharedPtr<Model> LoadGrid(float cellSize, int numCells);
      SharedPtr<Model> LoadBox();
      SharedPtr<Model> LoadQuad();

      Material GetDefaultMaterial();

      void SetInverseTranslation(bool inverse);

      Vk::DescriptorSetLayout* GetMeshTextureDescriptorSetLayout();
      Vk::DescriptorPool* GetMeshTextureDescriptorPool();

      static void SetFlipWindingOrder(bool flipWindingOrder);
      static bool GetFlipWindingOrder();
   private:
      std::map<std::string, SharedPtr<Model>> mModelMap;
      SharedPtr<Model> mPlaceholderModel = nullptr;
      Vk::Device* mDevice;

      SharedPtr<AssimpLoader> mAssimpLoader;
      SharedPtr<glTFLoader> mglTFLoader;
      static bool mFlipWindingOrder;

      // The descriptor set layout that contains the PBR textures used when rendering models.
      // It is expected to be used in multiple shaders so instead of relying on the 
      // shader reflection we manually create a layout that every shader
      // that will have per mesh textures shall use
      SharedPtr<Vk::DescriptorSetLayout> mMeshTexturesDescriptorSetLayout;
      SharedPtr<Vk::DescriptorPool> mMeshTexturesDescriptorPool;
   };

   ModelLoader& gModelLoader();
}
