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
   /* Based on https://github.com/SaschaWillems/Vulkan/tree/master/examples/gltfskinning */
   class glTFModel
   {
   public:

      struct glTFVertex
      {
         glm::vec3 pos;
         glm::vec3 normal;
         glm::vec2 uv;
         glm::vec3 color;
         glm::vec4 tangent;
         glm::vec4 jointIndices;
         glm::vec4 jointWeights;
      };

      struct Material
      {
         glm::vec4 baseColorFactor = glm::vec4(1.0f);
         uint32_t baseColorTextureIndex;
         uint32_t normalTextureIndex;
         SharedPtr<Vk::DescriptorSet> descriptorSet;
      };

      struct Primitive
      {
         uint32_t firstIndex;
         uint32_t firstVertex;
         uint32_t indexCount;
         uint32_t vertexCount;
         int32_t materialIndex;
         bool hasIndices;
      };

      struct Mesh
      {
         std::vector<Primitive> primitives;
      };

      struct Node
      {
         glm::mat4 GetLocalMatrix();

         std::vector<Node*> children;
         Node* parent;
         Mesh mesh;
         uint32_t index;
         int32_t skin = -1;
         glm::vec3 translation = glm::vec3(0.0f);
         glm::vec3 scale = glm::vec3(1.0f);
         glm::quat rotation = glm::quat();
         glm::mat4 matrix = glm::mat4();
      };

      // Todo: better name
      struct ShaderTexture
      {
         SharedPtr<Vk::Texture> texture;
      };

      struct Skin
      {
         std::string name;
         Node* skeletonRoot = nullptr;
         std::vector<glm::mat4> inverseBindMatrices;
         std::vector<Node*> joints;
         SharedPtr<Vk::Buffer> ssbo;
         SharedPtr<Vk::DescriptorSet> descriptorSet;
      };

      struct AnimationSampler
      {
         std::string interpolation;
         std::vector<float> inputs;
         std::vector<glm::vec4> outputsVec4;
      };

      struct AnimationChannel
      {
         std::string path;
         Node* node;
         uint32_t samplerIndex;
      };

      struct Animation
      {
         std::string name;
         std::vector<AnimationSampler> samplers;
         std::vector<AnimationChannel> channels;
         float start = std::numeric_limits<float>::max();
         float end = std::numeric_limits<float>::min();
         float currentTime = 0.0f;
      };

      glTFModel();
      ~glTFModel();

      void LoadFromFile(std::string filename, Vk::Device* device);

      void Render(Vk::CommandBuffer* commandBuffer, Vk::PipelineInterface* pipelineInterface);
      void RenderNode(Vk::CommandBuffer* commandBuffer, Vk::PipelineInterface* pipelineInterface, Node* node);
      void UpdateAnimation(float deltaTime);

   private:
      void LoadImages(tinygltf::Model& input);
      void LoadMaterials(tinygltf::Model& input);
      void LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent,
                    uint32_t nodeIndex, std::vector<uint32_t>& indexVector, std::vector<glTFVertex>& vertexBuffer);
      void LoadSkins(tinygltf::Model& input, Vk::Device* device);
      void LoadAnimations(tinygltf::Model& input);
      uint32_t AppendVertexData(const tinygltf::Model& input, const tinygltf::Primitive& glTFPrimitive,
                                std::vector<glTFVertex>& vertexBuffer);
      uint32_t AppendIndexData(const tinygltf::Model& input, const tinygltf::Primitive& glTFPrimitive,
                               std::vector<uint32_t>& indexVector, uint32_t vertexStart);
      void CreateDeviceBuffers(std::vector<uint32_t>& indexVector, std::vector<glTFVertex>& vertexVector, Vk::Device* device);
      void CreateTextureDescriptorSet(Vk::Device* device);
      void CreateSkinningDescriptorSet(Vk::Device* device);
      void UpdateJoints(Node* node);

      // Helper functions
      glm::mat4 GetNodeMatrix(Node* node);
      Node* NodeFromIndex(uint32_t index);
      Node* FindNode(Node* parent, uint32_t index);

   private:
      std::vector<ShaderTexture> mImages;
      std::vector<int32_t> mImageRefs;
      std::vector<Material> mMaterials;
      std::vector<Node*> mNodes;
      std::vector<Skin> mSkins;
      std::vector<Animation> mAnimations;
      std::string mFilename;
      SharedPtr<Vk::Buffer> mVertexBuffer = nullptr;
      SharedPtr<Vk::Buffer> mIndexBuffer = nullptr;
      uint32_t mIndicesCount;
      uint32_t mVerticesCount;
      uint32_t mActiveAnimation = 0;
      bool mHasSkin;

      // Todo: move these
      SharedPtr<Vk::DescriptorSetLayout> mMeshTexturesDescriptorSetLayout;
      SharedPtr<Vk::DescriptorSetLayout> mMeshSkinningDescriptorSetLayout;
      SharedPtr<Vk::DescriptorPool> mMeshTexturesDescriptorPool;
      SharedPtr<Vk::DescriptorPool> mMeshSkinningDescriptorPool;
   };
}
