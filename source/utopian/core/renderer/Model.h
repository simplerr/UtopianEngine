#pragma once

#include <core/components/CRigidBody.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "tinygltf/tiny_gltf.h"
#include "vulkan/Texture.h"
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/Vertex.h"
#include "core/renderer/Primitive.h"
#include "SkinAnimator.h"

namespace Utopian
{
   struct Material
   {
      glm::vec4 baseColorFactor = glm::vec4(1.0f);
      SharedPtr<Vk::Texture> colorTexture;
      SharedPtr<Vk::Texture> normalTexture;
      SharedPtr<Vk::Texture> specularTexture;
      SharedPtr<Vk::DescriptorSet> descriptorSet;
   };

   class Mesh
   {
   public:
      void AddPrimitive(Primitive* primitive, Material material) {
         primitives.push_back(primitive);
         materials.push_back(material);
      }

   //private:
      std::vector<Primitive*> primitives;
      std::vector<Material> materials;
   };

   struct RenderCommand
   {
      Mesh* mesh;
      VkDescriptorSet skinDescriptorSet;
      glm::mat4 world;
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
   class Model
   {
   public:
      Model();
      ~Model();

      void Init();

      void AddNode(Node* node);
      void AddSkinAnimator(SharedPtr<SkinAnimator> skinAnimator);

      /** Convenience function when working with simple models only containing a single primitive. */
      Primitive* GetFirstPrimitive();

      void GetRenderCommands(std::vector<RenderCommand>& renderCommands, glm::mat4 worldMatrix);
      void AppendRenderCommands(std::vector<RenderCommand>& renderCommands, Node* node, glm::mat4 worldMatrix);

      void UpdateAnimation(float deltaTime);
      bool IsAnimated() const;

      // Used by SkinAnimator to the node to animate
      Node* NodeFromIndex(uint32_t index);
      Node* FindNode(Node* parent, uint32_t index);

      BoundingBox GetBoundingBox();

   private:
      void DestroyNode(Node* node);

      // Todo: currently only returns the first primitive bounding box
      BoundingBox CalculateBoundingBox(Node* node, glm::mat4 world);

   private:
      std::vector<Node*> mNodes;
      Primitive* mFirstPrimitive = nullptr;
      SharedPtr<SkinAnimator> mSkinAnimator = nullptr;
      std::string mFilename;
      BoundingBox mBoundingBox;
   };
}
