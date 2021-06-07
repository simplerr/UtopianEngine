#include <stdio.h>
#include <glm/gtc/type_ptr.hpp>
#include <vulkan/vulkan_core.h>
#include "core/Log.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/PipelineInterface.h"
#include "glTFModel.h"

// Todo: remove
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/DescriptorSet.h"

#define TINYGLTF_IMPLEMENTATION
#include "tinygltf/tiny_gltf.h"

namespace Utopian
{
   glTFModel::glTFModel()
   {

   }

   glTFModel::~glTFModel()
   {

   }

   void glTFModel::LoadFromFile(std::string filename, Vk::Device* device)
   {
      mFilename = filename;

      tinygltf::Model glTFInput;
      tinygltf::TinyGLTF gltfContext;
      std::string error, warning;

      bool fileLoaded = gltfContext.LoadASCIIFromFile(&glTFInput, &error, &warning, filename);

      if (fileLoaded)
      {
         LoadImages(glTFInput);
         LoadMaterials(glTFInput);

         std::vector<uint32_t> indexVector;
         std::vector<Vk::Vertex> vertexVector;

         const tinygltf::Scene& scene = glTFInput.scenes[0];
         for (size_t i = 0; i < scene.nodes.size(); i++)
         {
            const tinygltf::Node node = glTFInput.nodes[scene.nodes[i]];
            LoadNode(node, glTFInput, nullptr, indexVector, vertexVector);
         }

         CreateDeviceBuffers(indexVector, vertexVector, device);
         CreateTextureDescriptorSet(device);
      }
      else
      {
         UTO_LOG("Failed to load glTF model " + filename);
      }
   }

   void glTFModel::LoadImages(tinygltf::Model& input)
   {
      for (size_t i = 0; i < input.images.size(); i++)
      {
         tinygltf::Image& glTFImage = input.images[i];

         ShaderTexture shaderTexture;

         shaderTexture.texture = Vk::gTextureLoader().CreateTexture(&glTFImage.image[0], VK_FORMAT_R8G8B8A8_UNORM,
                                                                    glTFImage.width, glTFImage.height, 1, sizeof(uint32_t));
         mImages.push_back(shaderTexture);
      }

      mImageRefs.resize(input.textures.size());
      for (size_t i = 0; i < input.textures.size(); i++)
      {
         mImageRefs[i] = input.textures[i].source;
      }
   }

   void glTFModel::LoadMaterials(tinygltf::Model& input)
   {
      mMaterials.resize(input.materials.size());

      for (size_t i = 0; i < input.materials.size(); i++)
      {
         tinygltf::Material glTFMaterial = input.materials[i];

         if (glTFMaterial.values.find("baseColorFactor") != glTFMaterial.values.end()) {
            mMaterials[i].baseColorFactor = glm::make_vec4(glTFMaterial.values["baseColorFactor"].ColorFactor().data());
         }
         if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end()) {
            mMaterials[i].baseColorTextureIndex = glTFMaterial.values["baseColorTexture"].TextureIndex();
         }
         if (glTFMaterial.additionalValues.find("normalTexture") != glTFMaterial.additionalValues.end()) {
            mMaterials[i].normalTextureIndex = glTFMaterial.additionalValues["normalTexture"].TextureIndex();
         }
      }
   }

   void glTFModel::LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent,
                            std::vector<uint32_t>& indexVector, std::vector<Vk::Vertex>& vertexVector)
   {
      Node node{};
      node.matrix = glm::mat4(1.0f);

      // Get the local node matrix
      // It's either made up from translation, rotation, scale or a 4x4 matrix
      if (inputNode.translation.size() == 3) {
         node.matrix = glm::translate(node.matrix, glm::vec3(glm::make_vec3(inputNode.translation.data())));
      }
      if (inputNode.rotation.size() == 4) {
         glm::quat q = glm::make_quat(inputNode.rotation.data());
         node.matrix *= glm::mat4(q);
      }
      if (inputNode.scale.size() == 3) {
         node.matrix = glm::scale(node.matrix, glm::vec3(glm::make_vec3(inputNode.scale.data())));
      }
      if (inputNode.matrix.size() == 16) {
         node.matrix = glm::make_mat4x4(inputNode.matrix.data());
      }

      // Load children
      if (inputNode.children.size() > 0) {
         for (size_t i = 0; i < inputNode.children.size(); i++) {
            LoadNode(input.nodes[inputNode.children[i]], input , &node, indexVector, vertexVector);
         }
      }

      if (inputNode.mesh > -1)
      {
         // Iterate through all primitives of this node's mesh
         const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];
         for (size_t i = 0; i < mesh.primitives.size(); i++)
         {
            uint32_t indexStart = static_cast<uint32_t>(indexVector.size());
            uint32_t vertexStart = static_cast<uint32_t>(vertexVector.size());

            const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
            AppendVertexData(input, glTFPrimitive, vertexVector);
            uint32_t indexCount = AppendIndexData(input, glTFPrimitive, indexVector, vertexStart);

            Primitive primitive{};
            primitive.firstIndex = indexStart;
            primitive.indexCount = indexCount;
            primitive.materialIndex = glTFPrimitive.material;
            node.mesh.primitives.push_back(primitive);
         }
      }

      if (parent) {
         parent->children.push_back(node);
      }
      else {
         mNodes.push_back(node);
      }
   }

   void glTFModel::AppendVertexData(const tinygltf::Model& input, const tinygltf::Primitive& glTFPrimitive,
                                    std::vector<Vk::Vertex>& vertexVector)
   {
      const float* positionBuffer = nullptr;
      const float* normalsBuffer = nullptr;
      const float* texCoordsBuffer = nullptr;
      const float* tangentsBuffer = nullptr;
      size_t vertexCount = 0;

      if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end())
      {
         const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("POSITION")->second];
         const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
         positionBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
         vertexCount = accessor.count;
      }
      if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end())
      {
         const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
         const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
         normalsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
      }
      // Get buffer data for vertex texture coordinates
      // glTF supports multiple sets, we only load the first one
      if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end())
      {
         const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
         const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
         texCoordsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
      }
      if (glTFPrimitive.attributes.find("TANGENT") != glTFPrimitive.attributes.end())
      {
         const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TANGENT")->second];
         const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
         tangentsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
      }

      // Append data to model's vertex buffer
      for (size_t v = 0; v < vertexCount; v++)
      {
         Vk::Vertex vert{};
         vert.Pos = glm::make_vec3(&positionBuffer[v * 3]);
         vert.Normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
         vert.Tex = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);

         // Todo: the tangent should be a vec4, the W-component stores handedness of the tangent
         vert.Tangent = tangentsBuffer ? glm::make_vec3(&tangentsBuffer[v * 4]) : glm::vec3(0.0f);
         vert.Color = glm::vec3(1.0f);
         vertexVector.push_back(vert);
      }
   }

   uint32_t glTFModel::AppendIndexData(const tinygltf::Model& input, const tinygltf::Primitive& glTFPrimitive,
                                       std::vector<uint32_t>& indexVector, uint32_t vertexStart)
   {
      const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.indices];
      const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
      const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];

      uint32_t indexCount = static_cast<uint32_t>(accessor.count);

      // glTF supports different component types of indices
      switch (accessor.componentType) {
      case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
         uint32_t* buf = new uint32_t[accessor.count];
         memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint32_t));
         for (size_t index = 0; index < accessor.count; index++) {
            indexVector.push_back(buf[index] + vertexStart);
         }
         break;
      }
      case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
         uint16_t* buf = new uint16_t[accessor.count];
         memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint16_t));
         for (size_t index = 0; index < accessor.count; index++) {
            indexVector.push_back(buf[index] + vertexStart);
         }
         break;
      }
      case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
         uint8_t* buf = new uint8_t[accessor.count];
         memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint8_t));
         for (size_t index = 0; index < accessor.count; index++) {
            indexVector.push_back(buf[index] + vertexStart);
         }
         break;
      }
      default:
         UTO_LOG("Index component type " + std::to_string(accessor.componentType) + " not supported!");
         assert(0);
      }

      return indexCount;
   }

   void glTFModel::CreateDeviceBuffers(std::vector<uint32_t>& indexVector, std::vector<Vk::Vertex>& vertexVector, Vk::Device* device)
   {
      mVerticesCount = (uint32_t)vertexVector.size();
      mIndicesCount = (uint32_t)indexVector.size();

      uint32_t vertexVectorSize = mVerticesCount * sizeof(Vk::Vertex);
      uint32_t indexVectorSize = mIndicesCount * sizeof(uint32_t);

      // Host visible staging buffers
      Vk::BUFFER_CREATE_INFO stagingVertexCI;
      stagingVertexCI.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      stagingVertexCI.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      stagingVertexCI.data = vertexVector.data();
      stagingVertexCI.size = vertexVectorSize;
      stagingVertexCI.name = "Staging Vertex buffer: " + mFilename;
      Vk::Buffer vertexStaging = Vk::Buffer(stagingVertexCI, device);

      Vk::BUFFER_CREATE_INFO stagingIndexCI;
      stagingIndexCI.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      stagingIndexCI.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      stagingIndexCI.data = indexVector.data();
      stagingIndexCI.size = indexVectorSize;
      stagingIndexCI.name = "Staging Index buffer: " + mFilename;
      Vk::Buffer indexStaging = Vk::Buffer(stagingIndexCI, device);

      // Device local target buffers
      Vk::BUFFER_CREATE_INFO vertexCI;
      vertexCI.usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
      vertexCI.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      vertexCI.data = nullptr;
      vertexCI.size = vertexVectorSize;
      vertexCI.name = "Vertex buffer: " + mFilename;
      mVertexBuffer = std::make_shared<Vk::Buffer>(vertexCI, device);

      Vk::BUFFER_CREATE_INFO indexCI;
      indexCI.usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
      indexCI.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      indexCI.data = nullptr;
      indexCI.size = indexVectorSize;
      indexCI.name = "Index buffer: " + mFilename;
      mIndexBuffer = std::make_shared<Vk::Buffer>(indexCI, device);

      // Copy from host visible to device local memory
      Vk::CommandBuffer cmdBuffer = Vk::CommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

      vertexStaging.Copy(&cmdBuffer, mVertexBuffer.get());
      indexStaging.Copy(&cmdBuffer, mIndexBuffer.get());

      cmdBuffer.Flush();
   }

   void glTFModel::CreateTextureDescriptorSet(Vk::Device* device)
   {
      // Todo: move these
      mMeshTexturesDescriptorSetLayout = std::make_shared<Vk::DescriptorSetLayout>(device);
      mMeshTexturesDescriptorSetLayout->AddCombinedImageSampler(0, VK_SHADER_STAGE_ALL, 1); // diffuseSampler
      mMeshTexturesDescriptorSetLayout->AddCombinedImageSampler(1, VK_SHADER_STAGE_ALL, 1); // normalSampler
      mMeshTexturesDescriptorSetLayout->Create();

      mMeshTexturesDescriptorPool = std::make_shared<Vk::DescriptorPool>(device);
      mMeshTexturesDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100);
      mMeshTexturesDescriptorPool->Create();

      for (auto& material : mMaterials)
      {
         Vk::Texture* colorTexture = mImages[material.baseColorTextureIndex].texture.get();
         Vk::Texture* normalTexture = mImages[material.normalTextureIndex].texture.get();

         material.descriptorSet = std::make_shared<Vk::DescriptorSet>(device, mMeshTexturesDescriptorSetLayout.get(),
                                                                      mMeshTexturesDescriptorPool.get());
         material.descriptorSet->BindCombinedImage(0, colorTexture->GetDescriptor());
         material.descriptorSet->BindCombinedImage(1, normalTexture->GetDescriptor());
         material.descriptorSet->UpdateDescriptorSets();
      }
   }

   void glTFModel::Render(Vk::CommandBuffer* commandBuffer, Vk::PipelineInterface* pipelineInterface)
   {
      // All primitives share the same vertex and index buffers
      commandBuffer->CmdBindVertexBuffer(0, 1, mVertexBuffer.get());
      commandBuffer->CmdBindIndexBuffer(mIndexBuffer.get(), 0, VK_INDEX_TYPE_UINT32);

      for (auto& node : mNodes) {
         RenderNode(commandBuffer, pipelineInterface, node);
      }
   }

   void glTFModel::RenderNode(Vk::CommandBuffer* commandBuffer, Vk::PipelineInterface* pipelineInterface, Node node)
   {
      if (node.mesh.primitives.size() > 0)
      {
         glm::mat4 nodeMatrix = node.matrix;

         // Todo: get parents matrices

         commandBuffer->CmdPushConstants(pipelineInterface, VK_SHADER_STAGE_ALL, sizeof(glm::mat4), &nodeMatrix);
         for (Primitive& primitive : node.mesh.primitives)
         {
            if (primitive.indexCount > 0)
            {
               int32_t imageRef = mImageRefs[mMaterials[primitive.materialIndex].baseColorTextureIndex];
               VkDescriptorSet descriptorSet = mMaterials[primitive.materialIndex].descriptorSet->GetVkHandle();
               commandBuffer->CmdBindDescriptorSet(pipelineInterface, 1, &descriptorSet, VK_PIPELINE_BIND_POINT_GRAPHICS, 1);
               commandBuffer->CmdDrawIndexed(primitive.indexCount, 1, primitive.firstIndex, 0, 0);
            }
         }
      }

      for (auto& child : node.children) {
         RenderNode(commandBuffer, pipelineInterface, child);
      }
   }
}
