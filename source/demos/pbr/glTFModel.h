#pragma once

#include <core/components/CRigidBody.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "tinygltf/tiny_gltf.h"
#include "vulkan/Texture.h"
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/Vertex.h"

namespace Utopian
{
   class glTFModel 
   {
   public:
      struct Material
      {
         glm::vec4 baseColorFactor = glm::vec4(1.0f);
         uint32_t baseColorTextureIndex;
      };

      struct Primitive
      {
         uint32_t firstIndex;
         uint32_t indexCount;
         int32_t materialIndex;
      };

      struct Mesh
      {
         std::vector<Primitive> primitives;
      };

      struct Node
      {
         Node* parent;
         std::vector<Node> children;
         Mesh mesh;
         glm::mat4 matrix;
      };

      // Todo: better name
      struct ShaderTexture
      {
         SharedPtr<Vk::Texture> texture;
         SharedPtr<Vk::DescriptorSet> descriptorSet;
      };

      glTFModel();
      ~glTFModel();

      void LoadFromFile(std::string filename, Vk::Device* device);

      void Render(Vk::CommandBuffer* commandBuffer, Vk::PipelineInterface* pipelineInterface);
      void RenderNode(Vk::CommandBuffer* commandBuffer, Vk::PipelineInterface* pipelineInterface, Node node);

   private:
      void LoadImages(tinygltf::Model& input);
      void LoadMaterials(tinygltf::Model& input);
      void LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent,
                    std::vector<uint32_t>& indexVector, std::vector<Vk::Vertex>& vertexBuffer);
      void AppendVertexData(const tinygltf::Model& input, const tinygltf::Primitive& glTFPrimitive,
                            std::vector<Vk::Vertex>& vertexBuffer);
      uint32_t AppendIndexData(const tinygltf::Model& input, const tinygltf::Primitive& glTFPrimitive,
                               std::vector<uint32_t>& indexVector, uint32_t vertexStart);
      void CreateDeviceBuffers(std::vector<uint32_t>& indexVector, std::vector<Vk::Vertex>& vertexVector, Vk::Device* device);
      void CreateTextureDescriptorSet(Vk::Device* device);

   private:
      std::vector<ShaderTexture> mImages;
      std::vector<int32_t> mImageRefs;
      std::vector<Material> mMaterials;
      std::vector<Node> mNodes;
      std::string mFilename;
      SharedPtr<Vk::Buffer> mVertexBuffer;
      SharedPtr<Vk::Buffer> mIndexBuffer;
      uint32_t mIndicesCount;
      uint32_t mVerticesCount;

      // Todo: move these
      SharedPtr<Vk::DescriptorSetLayout> mMeshTexturesDescriptorSetLayout;
      SharedPtr<Vk::DescriptorPool> mMeshTexturesDescriptorPool;
   };
}