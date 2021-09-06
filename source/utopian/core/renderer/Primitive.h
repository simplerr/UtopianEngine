#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/Vertex.h"
#include "vulkan/handles/Buffer.h"
#include "utility/math/BoundingBox.h"
#include "utility/Common.h"

#define DEFAULT_NORMAL_MAP_TEXTURE "data/textures/flat_normalmap.png"
#define DEFAULT_SPECULAR_MAP_TEXTURE "data/textures/default_specular_map.png"

namespace Utopian
{
   class Primitive
   {
   public:
      Primitive(Vk::Device* device);
      ~Primitive();

      void AddVertex(Vk::Vertex vertex);
      void AddVertex(glm::vec3 pos);
      void AddLine(uint32_t v1, uint32_t v2);
      void AddTriangle(uint32_t v1, uint32_t v2, uint32_t v3);
      void AddQuad(uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4);
      void BuildBuffers(Vk::Device* device);
      void BuildBuffers(const std::vector<Vk::Vertex>& vertices, std::vector<uint32_t>);

      void LoadTextures(std::string diffusePath, std::string normalPath = DEFAULT_NORMAL_MAP_TEXTURE, std::string specularPath = DEFAULT_SPECULAR_MAP_TEXTURE);

      void SetTexture(SharedPtr<Vk::Texture> texture);
      void SetSpecularTexture(SharedPtr<Vk::Texture> texture);
      VkDescriptorSet GetTextureDescriptorSet();

      BoundingBox GetBoundingBox();

      uint32_t GetNumIndices() const;
      uint32_t GetNumVertices() const;

      Vk::Buffer* GetVertxBuffer();
      Vk::Buffer* GetIndexBuffer();

      Vk::Texture* GetDiffuseTexture();
      Vk::Texture* GetNormalTexture();
      Vk::Texture* GetSpecularTexture();

      // Note: Do not call this every run iteration
      std::vector<Vk::Texture*> GetTextures();

      void SetDebugName(std::string debugName);

      std::vector<Vk::Vertex> vertexVector;
      std::vector<unsigned int> indexVector;
   private:
      // Creates a DescriptorSet from the diffuse and normal textures that was added to the mesh.
      void CreateDescriptorSets(SharedPtr<Vk::DescriptorSetLayout> descriptorSetLayout, SharedPtr<Vk::DescriptorPool> descriptorPool);

   private:
      SharedPtr<Vk::Buffer> mVertexBuffer;
      SharedPtr<Vk::Buffer> mIndexBuffer;
      SharedPtr<Vk::DescriptorSet> mTextureDescriptorSet;

      Vk::Device* mDevice;
      SharedPtr<Vk::Texture> mDiffuseTexture;
      SharedPtr<Vk::Texture> mNormalTexture;
      SharedPtr<Vk::Texture> mSpecularTexture;

      BoundingBox mBoundingBox;

      std::string mDebugName = "unnamed";
   };
}
