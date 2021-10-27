#pragma once

#include <core/components/CRigidBody.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "tinygltf/tiny_gltf.h"
#include "vulkan/Texture.h"
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/Vertex.h"
#include "vulkan/ShaderBuffer.h"
#include "core/renderer/Primitive.h"
#include "SkinAnimator.h"

#define JOINT_MATRICES_DESCRIPTOR_SET (2u)

namespace Utopian
{
   UNIFORM_BLOCK_BEGIN(MaterialProperties)
      UNIFORM_PARAM(glm::vec4, baseColorFactor)
      UNIFORM_PARAM(float, metallicFactor)
      UNIFORM_PARAM(float, roughnessFactor)
      UNIFORM_PARAM(float, occlusionFactor)
      UNIFORM_PARAM(float, pad)
   UNIFORM_BLOCK_END()

   struct Material
   {
      Material();

      void UpdateTextureDescriptors(Vk::Device* device);

      SharedPtr<MaterialProperties> properties = nullptr;
      SharedPtr<Vk::Texture> colorTexture;
      SharedPtr<Vk::Texture> normalTexture;
      SharedPtr<Vk::Texture> specularTexture;
      SharedPtr<Vk::Texture> metallicRoughnessTexture;
      SharedPtr<Vk::Texture> occlusionTexture;
      SharedPtr<Vk::DescriptorSet> descriptorSet;
      std::string name = "Material_unknown";
   };

   class Mesh
   {
   public:
      Mesh() {}
      Mesh(Primitive* primitive, Material* material) {
         AddPrimitive(primitive, material);
      }

      void AddPrimitive(Primitive* primitive, Material* material) {
         primitives.push_back(primitive);
         materials.push_back(material);
      }

      std::vector<Primitive*> primitives;
      std::vector<Material*> materials;
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
    * Represents all models in the engine.
    * Model can be viewed as a local scene graph containing a hierarchy of nodes.
    *
    * Models can be created manually or from the ModelLoader that has both
    * a glTF loader and an Assimp loader. Skinning is only supported when loaded
    * as .gltf.
    */
   class Model
   {
   public:
      Model();
      ~Model();

      void Init();

      Material* AddMaterial(const Material& material);
      Primitive* AddPrimitive(const Primitive& primitive);
      Node* CreateNode();

      void AddRootNode(Node* node);
      void AddSkinAnimator(SharedPtr<SkinAnimator> skinAnimator);

      Primitive* GetPrimitive(uint32_t index);
      Material* GetMaterial(uint32_t index);
      SkinAnimator* GetAnimator();

      void GetRenderCommands(std::vector<RenderCommand>& renderCommands, glm::mat4 worldMatrix);
      void AppendRenderCommands(std::vector<RenderCommand>& renderCommands, Node* node, glm::mat4 worldMatrix);

      void UpdateAnimation(float deltaTime);
      bool IsAnimated() const;

      void SetFilename(std::string filename);

      // Used by SkinAnimator to the node to animate
      Node* NodeFromIndex(uint32_t index);
      Node* FindNode(Node* parent, uint32_t index);

      BoundingBox GetBoundingBox();
      uint32_t GetNumPrimitives() const;
      uint32_t GetNumMaterials() const;

   private:
      // Todo: currently only returns the first primitive bounding box
      BoundingBox CalculateBoundingBox(Node* node, glm::mat4 world);

   private:
      std::vector<SharedPtr<Primitive>> mPrimitives;
      std::vector<SharedPtr<Material>> mMaterials;
      std::vector<SharedPtr<Node>> mNodes;
      std::vector<Node*> mRootNodes;
      SharedPtr<SkinAnimator> mSkinAnimator = nullptr;
      std::string mFilename;
      BoundingBox mBoundingBox;
   };
}
