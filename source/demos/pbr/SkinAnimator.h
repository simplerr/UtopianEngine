#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "tinygltf/tiny_gltf.h"
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"

namespace Utopian
{
   class glTFModel;
   struct Node;

   class SkinAnimator
   {
   public:
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

      SkinAnimator(tinygltf::Model& input, glTFModel* model, Vk::Device* device);
      ~SkinAnimator();

      void LoadSkins(tinygltf::Model& input, glTFModel* model, Vk::Device* device);
      void LoadAnimations(tinygltf::Model& input, glTFModel* model);

      VkDescriptorSet GetJointMatricesDescriptorSet(int32_t skin);

      void CreateSkinningDescriptorSet(Vk::Device* device);

      void UpdateAnimation(float deltaTime);
      void UpdateJoints(Node* node);
      glm::mat4 GetNodeMatrix(Node* node);

   private:
      std::vector<Skin> mSkins;
      std::vector<Animation> mAnimations;
      uint32_t mActiveAnimation = 0;

      // Todo: move
      SharedPtr<Vk::DescriptorSetLayout> mMeshSkinningDescriptorSetLayout;
      SharedPtr<Vk::DescriptorPool> mMeshSkinningDescriptorPool;
   };
}