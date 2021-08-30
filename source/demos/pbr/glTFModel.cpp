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

   void glTFModel::AddNode(Node* node)
   {
      mNodes.push_back(node);
   }

   void glTFModel::AddSkinAnimator(SharedPtr<SkinAnimator> skinAnimator)
   {
      mSkinAnimator = skinAnimator;

      // Calculate initial pose
      for (auto node : mNodes)
         mSkinAnimator->UpdateJoints(node);
   }

   void glTFModel::DestroyNode(Node* node)
   {
      for (auto& primitive : node->renderable.primitives)
      {
         delete primitive;
      }

      node->renderable.materials.clear();

      for (auto& child : node->children)
      {
        DestroyNode(child);
      }

      delete node;
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

   void glTFModel::GetRenderCommands(std::vector<RenderCommand>& renderCommands, glm::mat4 worldMatrix)
   {
      for (auto& node : mNodes) {
         AppendRenderCommands(renderCommands, node, worldMatrix);
      }
   }

   void glTFModel::AppendRenderCommands(std::vector<RenderCommand>& renderCommands, Node* node, glm::mat4 worldMatrix)
   {
      glm::mat4 nodeMatrix = worldMatrix * node->GetLocalMatrix();

      RenderCommand command;
      command.world = nodeMatrix;
      command.skinDescriptorSet = VK_NULL_HANDLE;

      if (node->renderable.primitives.size() > 0)
      {
         if (IsAnimated())
         {
            command.skinDescriptorSet = mSkinAnimator->GetJointMatricesDescriptorSet(node->skin);
         }

         command.renderable = &node->renderable;
         renderCommands.push_back(command);
      }

      for (auto& child : node->children) {
         AppendRenderCommands(renderCommands, child, nodeMatrix);
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
