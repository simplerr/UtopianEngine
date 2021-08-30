#pragma once
#include <string>
#include "tinygltf/tiny_gltf.h"
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"
#include "glTFModel.h"

namespace Utopian
{
   class glTFLoader
   {
   public:
      glTFLoader(Vk::Device* device);
      ~glTFLoader();

      SharedPtr<glTFModel> LoadModel(std::string filename, Vk::Device* device);

      Material2 GetDefaultMaterial();

   private:
      std::vector<Material2> LoadMaterials(tinygltf::Model& input, glTFModel* model);
      void LoadNode(glTFModel* model, const tinygltf::Node& inputNode, std::vector<Material2>& loadedMaterials,
                    const tinygltf::Model& input, Node* parent, uint32_t nodeIndex, Vk::Device* device);
      void AppendVertexData(const tinygltf::Model& input, const tinygltf::Primitive& glTFPrimitive, Vk::Mesh* primitive);
      void AppendIndexData(const tinygltf::Model& input, const tinygltf::Primitive& glTFPrimitive, Vk::Mesh* primitive);
      void CreateDescriptorPools();

   private:
      SharedPtr<Vk::DescriptorSetLayout> mMeshTexturesDescriptorSetLayout;
      SharedPtr<Vk::DescriptorSetLayout> mMeshSkinningDescriptorSetLayout;
      SharedPtr<Vk::DescriptorPool> mMeshTexturesDescriptorPool;
      SharedPtr<Vk::DescriptorPool> mMeshSkinningDescriptorPool;
      Vk::Device* mDevice;
   };
}