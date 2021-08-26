#include <stdio.h>
#include <glm/gtc/type_ptr.hpp>
#include <vulkan/vulkan_core.h>
#include "core/Log.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/handles/Buffer.h"
#include "glTFModel.h"

// Todo: remove
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/DescriptorSet.h"

#define TINYGLTF_IMPLEMENTATION
#include "tinygltf/tiny_gltf.h"

namespace Utopian
{
   glm::mat4 Node::GetLocalMatrix()
   {
      return glm::translate(glm::mat4(1.0f), translation) *
             glm::mat4(rotation) *
             glm::scale(glm::mat4(1.0f), scale) *
             matrix;
   }

   glTFModel::glTFModel()
   {

   }

   glTFModel::~glTFModel()
   {
      for (Node* node : mNodes)
      {
         DestroyNode(node);
      }
   }

   void glTFModel::DestroyNode(Node* node)
   {
      for (auto& primitive : node->mesh.primitives)
      {
         delete primitive;
      }

      for (auto& child : node->children)
      {
        DestroyNode(child);
      }
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

         const tinygltf::Scene& scene = glTFInput.scenes[0];
         for (size_t i = 0; i < scene.nodes.size(); i++)
         {
            const tinygltf::Node node = glTFInput.nodes[scene.nodes[i]];
            LoadNode(node, glTFInput, nullptr, scene.nodes[i], device);
         }

         if (glTFInput.skins.size() > 0)
         {
            mSkinAnimator = std::make_shared<SkinAnimator>(glTFInput, this, device);

            // Calculate initial pose
            for (auto node : mNodes)
               mSkinAnimator->UpdateJoints(node);
         }

         // Todo: move these
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
                            uint32_t nodeIndex, Vk::Device* device)
   {
      Node* node = new Node();
      node->parent = parent;
      node->matrix = glm::mat4(1.0f);
      node->index = nodeIndex;
      node->skin = inputNode.skin;
      node->name = inputNode.name;

      // Get the local node matrix
      // It's either made up from translation, rotation, scale or a 4x4 matrix
      if (inputNode.translation.size() == 3) {
         node->translation = glm::make_vec3(inputNode.translation.data());
      }
      if (inputNode.rotation.size() == 4) {
         glm::quat q = glm::make_quat(inputNode.rotation.data());
         node->rotation = glm::mat4(q);
      }
      if (inputNode.scale.size() == 3) {
         node->scale = glm::make_vec3(inputNode.scale.data());
      }
      if (inputNode.matrix.size() == 16) {
         node->matrix = glm::make_mat4x4(inputNode.matrix.data());
      }

      // Load children
      if (inputNode.children.size() > 0) {
         for (size_t i = 0; i < inputNode.children.size(); i++) {
            LoadNode(input.nodes[inputNode.children[i]], input , node, inputNode.children[i], device);
         }
      }

      if (inputNode.mesh > -1)
      {
         // Iterate through all primitives of this node's mesh
         const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];
         for (size_t i = 0; i < mesh.primitives.size(); i++)
         {
            const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
            Vk::Mesh* primitive = new Vk::Mesh(NULL); // Todo remove the Device* argument
            AppendVertexData(input, glTFPrimitive, primitive);

            bool hasIndices = (glTFPrimitive.indices != -1);
            if (hasIndices)
               AppendIndexData(input, glTFPrimitive, primitive);

            primitive->BuildBuffers(device);

            node->mesh.primitives.push_back(primitive);
            node->mesh.materials.push_back(glTFPrimitive.material);
         }
      }

      if (parent) {
         parent->children.push_back(node);
      }
      else {
         mNodes.push_back(node);
      }
   }

   glm::mat4 glTFModel::GetNodeMatrix(Node* node)
   {
      glm::mat4 nodeMatrix = node->GetLocalMatrix();
      Node* currentParent = node->parent;

      while (currentParent)
      {
         nodeMatrix = currentParent->GetLocalMatrix() * nodeMatrix;
         currentParent = currentParent->parent;
      }

      return nodeMatrix;
   }

   void glTFModel::AppendVertexData(const tinygltf::Model& input, const tinygltf::Primitive& glTFPrimitive,
                                        Vk::Mesh* primitive)
   {
      const float* positionBuffer = nullptr;
      const float* normalsBuffer = nullptr;
      const float* texCoordsBuffer = nullptr;
      const float* tangentsBuffer = nullptr;
      const uint16_t* jointIndicesBuffer = nullptr;
      const float* jointWeightsBuffer = nullptr;
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
      if (glTFPrimitive.attributes.find("JOINTS_0") != glTFPrimitive.attributes.end())
      {
         const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("JOINTS_0")->second];
         const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
         jointIndicesBuffer = reinterpret_cast<const uint16_t*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
      }
      if (glTFPrimitive.attributes.find("WEIGHTS_0") != glTFPrimitive.attributes.end())
      {
         const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("WEIGHTS_0")->second];
         const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
         jointWeightsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
      }

      bool hasSkin = (jointIndicesBuffer && jointWeightsBuffer);

      for (size_t v = 0; v < vertexCount; v++)
      {
         Vk::Vertex vert = {};
         vert.pos = glm::make_vec3(&positionBuffer[v * 3]);
         vert.normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
         vert.uv = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
         vert.tangent = tangentsBuffer ? glm::make_vec4(&tangentsBuffer[v * 4]) : glm::vec4(0.0f);
         vert.color = glm::vec3(1.0f);
         vert.jointIndices = hasSkin ? glm::vec4(glm::make_vec4(&jointIndicesBuffer[v * 4])) : glm::vec4(0.0f);
         vert.jointWeights = hasSkin ? glm::make_vec4(&jointWeightsBuffer[v * 4]) : glm::vec4(0.0f);
         primitive->AddVertex(vert);
      }

      if (normalsBuffer == nullptr)
      {
         UTO_LOG("Missing normals for model, calculating flat normals");
         for (size_t v = 0; v < primitive->GetNumVertices(); v += 3)
         {
            std::vector<Vk::Vertex>& vertexVector = primitive->vertexVector;
            glm::vec3 v1 = vertexVector[v].pos - vertexVector[v+1].pos;
            glm::vec3 v2 = vertexVector[v].pos - vertexVector[v+2].pos;
            glm::vec3 normal = glm::cross(v1, v2);
            vertexVector[v].normal = normal;
            vertexVector[v+1].normal = normal;
            vertexVector[v+2].normal = normal;
         }
      }
   }

   void glTFModel::AppendIndexData(const tinygltf::Model& input, const tinygltf::Primitive& glTFPrimitive,
                                       Vk::Mesh* primitive)
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
         for (size_t index = 0; index < accessor.count; index += 3) {
            primitive->AddTriangle(buf[index], buf[index+1], buf[index+2]);
         }
         break;
      }
      case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
         uint16_t* buf = new uint16_t[accessor.count];
         memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint16_t));
         for (size_t index = 0; index < accessor.count; index += 3) {
            primitive->AddTriangle(buf[index], buf[index+1], buf[index+2]);
         }
         break;
      }
      case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
         uint8_t* buf = new uint8_t[accessor.count];
         memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint8_t));
         for (size_t index = 0; index < accessor.count; index += 3) {
            primitive->AddTriangle(buf[index], buf[index+1], buf[index+2]);
         }
         break;
      }
      default:
         UTO_LOG("Index component type " + std::to_string(accessor.componentType) + " not supported!");
         assert(0);
      }
   }

   void glTFModel::CreateTextureDescriptorSet(Vk::Device* device)
   {
      // Todo: move these
      mMeshTexturesDescriptorSetLayout = std::make_shared<Vk::DescriptorSetLayout>(device);
      mMeshTexturesDescriptorSetLayout->AddCombinedImageSampler(0, VK_SHADER_STAGE_ALL, 1); // diffuseSampler
      mMeshTexturesDescriptorSetLayout->AddCombinedImageSampler(1, VK_SHADER_STAGE_ALL, 1); // normalSampler
      mMeshTexturesDescriptorSetLayout->Create();

      mMeshTexturesDescriptorPool = std::make_shared<Vk::DescriptorPool>(device);
      mMeshTexturesDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 200);
      mMeshTexturesDescriptorPool->Create();

      for (auto& material : mMaterials)
      {
         Vk::Texture* colorTexture = mImages[mImageRefs[material.baseColorTextureIndex]].texture.get();
         Vk::Texture* normalTexture = mImages[mImageRefs[material.normalTextureIndex]].texture.get();

         material.descriptorSet = std::make_shared<Vk::DescriptorSet>(device, mMeshTexturesDescriptorSetLayout.get(),
                                                                      mMeshTexturesDescriptorPool.get());
         material.descriptorSet->BindCombinedImage(0, colorTexture->GetDescriptor());
         material.descriptorSet->BindCombinedImage(1, normalTexture->GetDescriptor());
         material.descriptorSet->UpdateDescriptorSets();
      }
   }

   void glTFModel::UpdateAnimation(float deltaTime)
   {
      if (IsAnimated())
      {
         mSkinAnimator->UpdateAnimation(deltaTime);

         for (auto node : mNodes)
            mSkinAnimator->UpdateJoints(node);
      }
   }

   void glTFModel::Render(Vk::CommandBuffer* commandBuffer, Vk::PipelineInterface* pipelineInterface, glm::mat4 worldMatrix)
   {
      for (auto& node : mNodes) {
         RenderNode(commandBuffer, pipelineInterface, node, worldMatrix);
      }
   }

   void glTFModel::RenderNode(Vk::CommandBuffer* commandBuffer, Vk::PipelineInterface* pipelineInterface, Node* node, glm::mat4 worldMatrix)
   {
      if (node->mesh.primitives.size() > 0)
      {
         glm::mat4 nodeMatrix = worldMatrix * node->GetLocalMatrix();

         // Todo: get parents matrices

         if (IsAnimated())
         {
            VkDescriptorSet descriptorSet = mSkinAnimator->GetJointMatricesDescriptorSet(node->skin);
            commandBuffer->CmdBindDescriptorSet(pipelineInterface, 1, &descriptorSet, VK_PIPELINE_BIND_POINT_GRAPHICS, 2);
         }

         commandBuffer->CmdPushConstants(pipelineInterface, VK_SHADER_STAGE_ALL, sizeof(glm::mat4), &nodeMatrix);
         for (uint32_t i = 0; i < node->mesh.primitives.size(); i++)
         {
            Vk::Mesh* primitive = node->mesh.primitives[i];

            if (primitive->GetNumIndices() > 0 || primitive->GetNumVertices() > 0)
            {
               VkDescriptorSet descriptorSet = mMaterials[node->mesh.materials[i]].descriptorSet->GetVkHandle();
               commandBuffer->CmdBindDescriptorSet(pipelineInterface, 1, &descriptorSet, VK_PIPELINE_BIND_POINT_GRAPHICS, 1);
               commandBuffer->CmdBindVertexBuffer(0, 1, primitive->GetVertxBuffer());

               if (primitive->GetNumIndices() > 0)
               {
                  commandBuffer->CmdBindIndexBuffer(primitive->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
                  commandBuffer->CmdDrawIndexed(primitive->GetNumIndices(), 1, 0, 0, 0);
               }
               else
                  commandBuffer->CmdDraw(primitive->GetNumVertices(), 1, 0, 0);
            }
         }
      }

      for (auto& child : node->children) {
         RenderNode(commandBuffer, pipelineInterface, child, worldMatrix);
      }
   }

   Node* glTFModel::FindNode(Node* parent, uint32_t index)
   {
      Node* nodeFound = nullptr;

      if (parent->index == index)
      {
         return parent;
      }
      for (auto &child : parent->children)
      {
         nodeFound = FindNode(child, index);
         if (nodeFound)
         {
            break;
         }
      }

      return nodeFound;
   }

   Node* glTFModel::NodeFromIndex(uint32_t index)
   {
      Node *nodeFound = nullptr;

      for (auto &node : mNodes)
      {
         nodeFound = FindNode(node, index);
         if (nodeFound)
         {
            break;
         }
      }

      return nodeFound;
   }

   bool glTFModel::IsAnimated() const
   {
      return (mSkinAnimator != nullptr);
   }
}
