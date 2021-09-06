#include <vulkan/vulkan_core.h>
#include <glm/gtc/matrix_transform.hpp>
#include "core/ModelLoader.h"
#include "core/renderer/Primitive.h"
#include "vulkan/handles/Device.h"
#include "vulkan/Debug.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/Texture.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/CommandBuffer.h"

namespace Utopian
{
   Primitive::Primitive(Vk::Device* device)
   {
      mDevice = device;
      mDiffuseTexture = nullptr;
      mNormalTexture = nullptr;
      mSpecularTexture = nullptr;
   }

   Primitive::~Primitive()
   {
   }

   void Primitive::AddVertex(Vk::Vertex vertex)
   {
      vertexVector.push_back(vertex);
   }

   void Primitive::AddVertex(glm::vec3 pos)
   {
      AddVertex(Vk::Vertex(pos));
   }

   void Primitive::AddLine(uint32_t v1, uint32_t v2)
   {
      indexVector.push_back(v1);
      indexVector.push_back(v2);
   }

   void Primitive::AddTriangle(uint32_t v1, uint32_t v2, uint32_t v3)
   {
      indexVector.push_back(v1);
      indexVector.push_back(v2);
      indexVector.push_back(v3);
   }

   void Primitive::AddQuad(uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4)
   {
      indexVector.push_back(v1);
      indexVector.push_back(v2);
      indexVector.push_back(v3);
      indexVector.push_back(v4);
   }

   void Primitive::BuildBuffers(Vk::Device* device)
   {
      uint32_t vertexBufferSize = GetNumVertices() * sizeof(Vk::Vertex);
      uint32_t indexBufferSize = GetNumIndices() * sizeof(uint32_t);

      Vk::BUFFER_CREATE_INFO stagingVertexCI;
      stagingVertexCI.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      stagingVertexCI.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      stagingVertexCI.data = vertexVector.data();
      stagingVertexCI.size = vertexBufferSize;
      stagingVertexCI.name = "Staging Vertex buffer: " + mDebugName;
      Vk::Buffer vertexStaging = Vk::Buffer(stagingVertexCI, device);

      Vk::BUFFER_CREATE_INFO vertexCI;
      vertexCI.usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
      vertexCI.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      vertexCI.data = nullptr;
      vertexCI.size = vertexBufferSize;
      vertexCI.name = "Vertex buffer: " + mDebugName;
      mVertexBuffer = std::make_shared<Vk::Buffer>(vertexCI, device);

      // Copy from host visible to device local memory
      Vk::CommandBuffer cmdBuffer = Vk::CommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
      vertexStaging.Copy(&cmdBuffer, mVertexBuffer.get());
      cmdBuffer.Flush();

      if (GetNumIndices() > 0)
      {
         Vk::BUFFER_CREATE_INFO stagingIndexCI;
         stagingIndexCI.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
         stagingIndexCI.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
         stagingIndexCI.data = indexVector.data();
         stagingIndexCI.size = indexBufferSize;
         stagingIndexCI.name = "Staging Index buffer: " + mDebugName;
         Vk::Buffer indexStaging = Vk::Buffer(stagingIndexCI, device);

         Vk::BUFFER_CREATE_INFO indexCI;
         indexCI.usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
         indexCI.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
         indexCI.data = nullptr;
         indexCI.size = indexBufferSize;
         indexCI.name = "Index buffer: " + mDebugName;
         mIndexBuffer = std::make_shared<Vk::Buffer>(indexCI, device);

         cmdBuffer.Begin();
         indexStaging.Copy(&cmdBuffer, mIndexBuffer.get());
         cmdBuffer.Flush();
      }
   }

   void Primitive::BuildBuffers(const std::vector<Vk::Vertex>& vertices, std::vector<uint32_t>)
   {
   }

   void Primitive::SetTexture(SharedPtr<Vk::Texture> texture)
   {
      mDiffuseTexture = texture;
      CreateDescriptorSets(Vk::gModelLoader().GetMeshTextureDescriptorSetLayout(), Vk::gModelLoader().GetMeshTextureDescriptorPool());
   }

   void Primitive::SetSpecularTexture(SharedPtr<Vk::Texture> texture)
   {
      mSpecularTexture = texture;
      CreateDescriptorSets(Vk::gModelLoader().GetMeshTextureDescriptorSetLayout(), Vk::gModelLoader().GetMeshTextureDescriptorPool());
   }

   VkDescriptorSet Primitive::GetTextureDescriptorSet()
   {
      return mTextureDescriptorSet->GetVkHandle();
   }

   void Primitive::CreateDescriptorSets(SharedPtr<Vk::DescriptorSetLayout> descriptorSetLayout, SharedPtr<Vk::DescriptorPool> descriptorPool)
   {
      mTextureDescriptorSet = std::make_shared<Vk::DescriptorSet>(mDevice, descriptorSetLayout.get(), descriptorPool.get());
      mTextureDescriptorSet->BindCombinedImage(0, mDiffuseTexture->GetDescriptor());
      mTextureDescriptorSet->BindCombinedImage(1, mNormalTexture->GetDescriptor());
      mTextureDescriptorSet->BindCombinedImage(2, mSpecularTexture->GetDescriptor());
      mTextureDescriptorSet->UpdateDescriptorSets();
   }

   BoundingBox Primitive::GetBoundingBox()
   {
      return mBoundingBox;
   }

   uint32_t Primitive::GetNumVertices() const
   {
      return vertexVector.size();
   }

   uint32_t Primitive::GetNumIndices() const
   {
      return indexVector.size();
   }

   Vk::Buffer* Primitive::GetVertxBuffer()
   {
      return mVertexBuffer.get();
   }

   Vk::Buffer* Primitive::GetIndexBuffer()
   {
      return mIndexBuffer.get();
   }

   Vk::Texture* Primitive::GetDiffuseTexture()
   {
       return mDiffuseTexture.get(); 
   }

   Vk::Texture* Primitive::GetNormalTexture()
   {
      return mNormalTexture.get();
   }

   Vk::Texture* Primitive::GetSpecularTexture()
   {
      return mSpecularTexture.get();
   };

   std::vector<Vk::Texture*> Primitive::GetTextures()
   {
      std::vector<Vk::Texture*> textures;
      textures.push_back(mDiffuseTexture.get());
      textures.push_back(mNormalTexture.get());
      textures.push_back(mSpecularTexture.get());

      return textures;
   }

   void Primitive::LoadTextures(std::string diffusePath, std::string normalPath, std::string specularPath)
   {
      mDiffuseTexture = Vk::gTextureLoader().LoadTexture(diffusePath);
      mNormalTexture = Vk::gTextureLoader().LoadTexture(normalPath);
      mSpecularTexture = Vk::gTextureLoader().LoadTexture(specularPath);
      CreateDescriptorSets(Vk::gModelLoader().GetMeshTextureDescriptorSetLayout(), Vk::gModelLoader().GetMeshTextureDescriptorPool());
   }

   void Primitive::SetDebugName(std::string debugName)
   {
      mDebugName = debugName;
   }
}
