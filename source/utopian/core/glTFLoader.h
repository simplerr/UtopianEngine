#pragma once
#include <string>
#include "tinygltf/tiny_gltf.h"
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"
#include "core/renderer/Model.h"

namespace Utopian
{
   /**
    * The loader used for loading .gltf models.
    * Supports PBR materials and skinning.
    */
   class glTFLoader
   {
   public:
      glTFLoader(Vk::Device* device);
      ~glTFLoader();

      SharedPtr<Model> LoadModel(std::string filename, Vk::Device* device);

      Material GetDefaultMaterial();

      // Todo: this is due to the coordinate system being inversed in the engine,
      // needs to be fixed.
      void SetInverseTranslation(bool inverse);

   private:
      void LoadMaterials(tinygltf::Model& input, Model* model);
      void LoadNode(Model* model, const tinygltf::Node& inputNode, const tinygltf::Model& input,
                    Node* parent, uint32_t nodeIndex, Vk::Device* device);
      void AppendVertexData(const tinygltf::Model& input, const tinygltf::Primitive& glTFPrimitive, Primitive* primitive);
      void AppendIndexData(const tinygltf::Model& input, const tinygltf::Primitive& glTFPrimitive, Primitive* primitive);
      void CreateDescriptorPools();

   private:
      SharedPtr<Vk::DescriptorSetLayout> mMeshSkinningDescriptorSetLayout;
      SharedPtr<Vk::DescriptorPool> mMeshSkinningDescriptorPool;
      Vk::Device* mDevice;
      bool mInverseTranslation = true;
   };
}
