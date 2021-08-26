#pragma once

#include <core/components/CRigidBody.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "tinygltf/tiny_gltf.h"
#include "vulkan/Texture.h"
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/Vertex.h"
#include "vulkan/Mesh.h"
#include "SkinAnimator.h"

namespace Utopian
{
   struct Mesh
   {
      std::vector<Vk::Mesh*> primitives;
      std::vector<int32_t> materials;
   };

   struct Node
   {
      glm::mat4 GetLocalMatrix();

      std::vector<Node*> children;
      std::string name;
      Node* parent;
      Mesh mesh;
      uint32_t index;
      int32_t skin = -1;
      glm::vec3 translation = glm::vec3(0.0f);
      glm::vec3 scale = glm::vec3(1.0f);
      glm::quat rotation = glm::quat();
      glm::mat4 matrix = glm::mat4();
   };

   /* Based on https://github.com/SaschaWillems/Vulkan/tree/master/examples/gltfskinning */
   class glTFModel
   {
   public:

      struct Material
      {
         glm::vec4 baseColorFactor = glm::vec4(1.0f);
         uint32_t baseColorTextureIndex;
         uint32_t normalTextureIndex;
         SharedPtr<Vk::DescriptorSet> descriptorSet;
      };

      // Todo: better name
      struct ShaderTexture
      {
         SharedPtr<Vk::Texture> texture;
      };

      glTFModel();
      ~glTFModel();

      void LoadFromFile(std::string filename, Vk::Device* device);

      void Render(Vk::CommandBuffer* commandBuffer, Vk::PipelineInterface* pipelineInterface, glm::mat4 worldMatrix);
      void RenderNode(Vk::CommandBuffer* commandBuffer, Vk::PipelineInterface* pipelineInterface, Node* node, glm::mat4 worldMatrix);
      void UpdateAnimation(float deltaTime);
      bool IsAnimated() const;

      Node* NodeFromIndex(uint32_t index);
      Node* FindNode(Node* parent, uint32_t index);

   private:
      void DestroyNode(Node* node);
      void LoadImages(tinygltf::Model& input);
      void LoadMaterials(tinygltf::Model& input);
      void LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent,
                    uint32_t nodeIndex, Vk::Device* device);
      void AppendVertexData(const tinygltf::Model& input, const tinygltf::Primitive& glTFPrimitive,
                            Vk::Mesh* primitive);
      void AppendIndexData(const tinygltf::Model& input, const tinygltf::Primitive& glTFPrimitive,
                           Vk::Mesh* primitive);
      void CreateTextureDescriptorSet(Vk::Device* device);

      // Helper functions
      glm::mat4 GetNodeMatrix(Node* node);

   private:
      std::vector<ShaderTexture> mImages;
      std::vector<int32_t> mImageRefs;
      std::vector<Material> mMaterials;
      std::vector<Node*> mNodes;
      std::string mFilename;
      uint32_t mActiveAnimation = 0;

      SharedPtr<SkinAnimator> mSkinAnimator = nullptr;

      // Todo: move these
      SharedPtr<Vk::DescriptorSetLayout> mMeshTexturesDescriptorSetLayout;
      SharedPtr<Vk::DescriptorPool> mMeshTexturesDescriptorPool;
   };
}
