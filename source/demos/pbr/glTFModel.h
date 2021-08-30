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
   struct Material2
   {
      glm::vec4 baseColorFactor = glm::vec4(1.0f);
      SharedPtr<Vk::Texture> colorTexture;
      SharedPtr<Vk::Texture> normalTexture;
      SharedPtr<Vk::DescriptorSet> descriptorSet;
   };

   class Renderable2
   {
   public:
      void AddPrimitive(Vk::Mesh* primitive, Material2 material) {
         primitives.push_back(primitive);
         materials.push_back(material);
      }

   //private:
      std::vector<Vk::Mesh*> primitives;
      std::vector<Material2> materials;
   };

   struct RenderCommand
   {
      Renderable2* renderable;
      VkDescriptorSet skinDescriptorSet;
      glm::mat4 world;
   };

   struct Node
   {
      glm::mat4 GetLocalMatrix();

      std::vector<Node*> children;
      std::string name;
      Node* parent;
      Renderable2 renderable;
      uint32_t index;
      int32_t skin = -1;
      glm::vec3 translation = glm::vec3(0.0f);
      glm::vec3 scale = glm::vec3(1.0f);
      glm::quat rotation = glm::quat();
      glm::mat4 matrix = glm::mat4();
   };

   /**
    * Represents all models in the engine, based on the glTF model format.
    * It can be viewed as a local scene graph containing a hierarchy of nodes.
    *
    * Models can be created manually or from the ModelLoader that has both
    * a glTF loader and an Assimp loader. Skinning is only supported when loaded
    * as .gltf.
    *
    * Example of manual model creation:
    *
    * Primitive* primitive = new Primitive();
    * primitive->AddVertex(glm::vec3(-0.5f, -0.5f, 0.5f));
    * primitive->AddVertex(glm::vec3(0.5f, -0.5f, 0.5f));
    * primitive->AddVertex(glm::vec3(0.5f, 0.5f, 0.5f));
    * primitive->AddVertex(glm::vec3(-0.5f, 0.5f, 0.5f));
    * primitive->AddTriangle(1, 2, 0);
    * primitive->AddTriangle(3, 0, 2);
    * primitive->BuildBuffers(device);
    *
    * Renderable renderable;
    * renderable.AddPrimitive(primitive, gEngine().DefaultMaterial());
    *
    * Node* node = new Node();
    * node->SetRenderable(renderable);
    *
    * Model* model = new Model();
    * model->AddNode(node);
    */
   class glTFModel
   {
   public:
      glTFModel();
      ~glTFModel();

      void AddNode(Node* node);
      void AddSkinAnimator(SharedPtr<SkinAnimator> skinAnimator);

      void GetRenderCommands(std::vector<RenderCommand>& renderCommands, glm::mat4 worldMatrix);
      void AppendRenderCommands(std::vector<RenderCommand>& renderCommands, Node* node, glm::mat4 worldMatrix);

      void UpdateAnimation(float deltaTime);
      bool IsAnimated() const;

      // Used by SkinAnimator to the node to animate
      Node* NodeFromIndex(uint32_t index);
      Node* FindNode(Node* parent, uint32_t index);

   private:
      void DestroyNode(Node* node);

   private:
      std::vector<Node*> mNodes;
      SharedPtr<SkinAnimator> mSkinAnimator = nullptr;
      std::string mFilename;
   };
}
