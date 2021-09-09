#include <stdio.h>
#include <glm/gtc/type_ptr.hpp>
#include <vulkan/vulkan_core.h>
#include "core/Log.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/handles/Buffer.h"
#include "Model.h"

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

   Model::Model()
   {

   }

   Model::~Model()
   {
      for (Node* node : mNodes)
      {
         DestroyNode(node);
      }
   }

   void Model::Init()
   {
      for (auto& node : mNodes)
      {
         mBoundingBox = CalculateBoundingBox(node, glm::mat4());
      }
   }

   BoundingBox Model::CalculateBoundingBox(Node* node, glm::mat4 world)
   {
      glm::mat4 nodeMatrix = world * node->GetLocalMatrix();
      BoundingBox bb;

      if (node->mesh.primitives.size() > 0)
         return node->mesh.primitives[0]->GetBoundingBox();

      for (auto& child : node->children)
      {
         bb = CalculateBoundingBox(child, nodeMatrix);
      }

      return bb;
   }

   void Model::AddNode(Node* node)
   {
      mNodes.push_back(node);

      if (mFirstPrimitive == nullptr && node->mesh.primitives.size() > 0)
      {
         mFirstPrimitive = node->mesh.primitives[0];
      }
   }

   void Model::AddSkinAnimator(SharedPtr<SkinAnimator> skinAnimator)
   {
      mSkinAnimator = skinAnimator;

      // Calculate initial pose
      for (auto node : mNodes)
         mSkinAnimator->UpdateJoints(node);
   }

   void Model::DestroyNode(Node* node)
   {
      for (auto& primitive : node->mesh.primitives)
      {
         delete primitive;
      }

      node->mesh.materials.clear();

      for (auto& child : node->children)
      {
        DestroyNode(child);
      }

      delete node;
   }

   void Model::UpdateAnimation(float deltaTime)
   {
      if (IsAnimated())
      {
         mSkinAnimator->UpdateAnimation(deltaTime);

         for (auto node : mNodes)
            mSkinAnimator->UpdateJoints(node);
      }
   }

   Primitive* Model::GetFirstPrimitive()
   {
      return mFirstPrimitive;
   }

   void Model::GetRenderCommands(std::vector<RenderCommand>& renderCommands, glm::mat4 worldMatrix)
   {
      for (auto& node : mNodes) {
         AppendRenderCommands(renderCommands, node, worldMatrix);
      }
   }

   void Model::AppendRenderCommands(std::vector<RenderCommand>& renderCommands, Node* node, glm::mat4 worldMatrix)
   {
      glm::mat4 nodeMatrix = worldMatrix * node->GetLocalMatrix();

      RenderCommand command;
      command.world = nodeMatrix;
      command.skinDescriptorSet = VK_NULL_HANDLE;

      if (node->mesh.primitives.size() > 0)
      {
         if (IsAnimated())
         {
            command.skinDescriptorSet = mSkinAnimator->GetJointMatricesDescriptorSet(node->skin);
         }

         command.mesh = &node->mesh;
         renderCommands.push_back(command);
      }

      for (auto& child : node->children) {
         AppendRenderCommands(renderCommands, child, nodeMatrix);
      }
   }

   Node* Model::FindNode(Node* parent, uint32_t index)
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

   Node* Model::NodeFromIndex(uint32_t index)
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

   bool Model::IsAnimated() const
   {
      return (mSkinAnimator != nullptr);
   }

   BoundingBox Model::GetBoundingBox()
   {
      return mBoundingBox;
   }
}
