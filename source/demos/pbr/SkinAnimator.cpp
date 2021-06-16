#include "SkinAnimator.h"
#include "glTFModel.h"
#include "core/Log.h"
#include "vulkan/handles/Buffer.h"

// Todo: remove
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/DescriptorSet.h"

namespace Utopian
{
   SkinAnimator::SkinAnimator(tinygltf::Model& input, glTFModel* model, Vk::Device* device)
   {
      LoadSkins(input, model, device);
      LoadAnimations(input, model);

      // Todo: remove
      CreateSkinningDescriptorSet(device);
   }

   SkinAnimator::~SkinAnimator()
   {

   }

   void SkinAnimator::LoadSkins(tinygltf::Model& input, glTFModel* model, Vk::Device* device)
   {
      mSkins.resize(input.skins.size());

      for (size_t i = 0; i < input.skins.size(); i++)
      {
         tinygltf::Skin glTFSkin = input.skins[i];

         mSkins[i].name = glTFSkin.name;
         mSkins[i].skeletonRoot = model->NodeFromIndex(glTFSkin.skeleton);

         // Find joint nodes
         for (int jointIndex : glTFSkin.joints)
         {
            Node* node = model->NodeFromIndex(jointIndex);
            if (node)
            {
               mSkins[i].joints.push_back(node);
            }
         }

         // Get the inverse bind matrices from the buffer associated to this skin
         if (glTFSkin.inverseBindMatrices > -1)
         {
            const tinygltf::Accessor& accessor = input.accessors[glTFSkin.inverseBindMatrices];
            const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];
            mSkins[i].inverseBindMatrices.resize(accessor.count);
            memcpy(mSkins[i].inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset],
                   accessor.count * sizeof(glm::mat4));

            Vk::BUFFER_CREATE_INFO createInfo;
            createInfo.usageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            createInfo.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            createInfo.data = mSkins[i].inverseBindMatrices.data();
            createInfo.size = sizeof(glm::mat4) * mSkins[i].inverseBindMatrices.size();
            createInfo.name = "SSBO jointMatrices " + mSkins[i].name;
            mSkins[i].ssbo = std::make_shared<Vk::Buffer>(createInfo, device);
         }
      }
   }

   void SkinAnimator::LoadAnimations(tinygltf::Model& input, glTFModel* model)
   {
      mAnimations.resize(input.animations.size());

      for (size_t i = 0; i < input.animations.size(); i++)
      {
         tinygltf::Animation glTFAnimation = input.animations[i];
         mAnimations[i].name = glTFAnimation.name;

         // Samplers
         mAnimations[i].samplers.resize(glTFAnimation.samplers.size());
         for (size_t j = 0; j < glTFAnimation.samplers.size(); j++)
         {
            tinygltf::AnimationSampler glTFSampler = glTFAnimation.samplers[j];
            AnimationSampler& dstSampler = mAnimations[i].samplers[j];
            dstSampler.interpolation = glTFSampler.interpolation;

            // Read sampler keyframe input time values
            {
               const tinygltf::Accessor& accessor = input.accessors[glTFSampler.input];
               const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
               const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];
               const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
               const float* buf = static_cast<const float *>(dataPtr);
               for (size_t index = 0; index < accessor.count; index++)
               {
                  dstSampler.inputs.push_back(buf[index]);
               }
               // Adjust animation's start and end times
               for (auto input : mAnimations[i].samplers[j].inputs)
               {
                  if (input < mAnimations[i].start)
                  {
                     mAnimations[i].start = input;
                  }
                  if (input > mAnimations[i].end)
                  {
                     mAnimations[i].end = input;
                  }
               }
            }

            // Read sampler keyframe output translate/rotate/scale values
            {
               const tinygltf::Accessor& accessor = input.accessors[glTFSampler.output];
               const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
               const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];
               const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
               switch (accessor.type)
               {
                  case TINYGLTF_TYPE_VEC3: {
                     const glm::vec3 *buf = static_cast<const glm::vec3 *>(dataPtr);
                     for (size_t index = 0; index < accessor.count; index++)
                     {
                        dstSampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));
                     }
                     break;
                  }
                  case TINYGLTF_TYPE_VEC4: {
                     const glm::vec4 *buf = static_cast<const glm::vec4 *>(dataPtr);
                     for (size_t index = 0; index < accessor.count; index++)
                     {
                        dstSampler.outputsVec4.push_back(buf[index]);
                     }
                     break;
                  }
                  default: {
                     UTO_LOG("Unknown type");
                     break;
                  }
               }
            }
         }

         // Channels
         mAnimations[i].channels.resize(glTFAnimation.channels.size());
         for (size_t j = 0; j < glTFAnimation.channels.size(); j++)
         {
            tinygltf::AnimationChannel glTFChannel = glTFAnimation.channels[j];
            AnimationChannel& dstChannel = mAnimations[i].channels[j];
            dstChannel.path = glTFChannel.target_path;
            dstChannel.samplerIndex = glTFChannel.sampler;
            dstChannel.node = model->NodeFromIndex(glTFChannel.target_node);
         }
      }
   }

   void SkinAnimator::CreateSkinningDescriptorSet(Vk::Device* device)
   {
      mMeshSkinningDescriptorSetLayout = std::make_shared<Vk::DescriptorSetLayout>(device);
      mMeshSkinningDescriptorSetLayout->AddStorageBuffer(0, VK_SHADER_STAGE_ALL, 1); // jointMatrices
      mMeshSkinningDescriptorSetLayout->Create();

      mMeshSkinningDescriptorPool = std::make_shared<Vk::DescriptorPool>(device);
      mMeshSkinningDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100);
      mMeshSkinningDescriptorPool->Create();

      for (auto& skin : mSkins)
      {
         VkDescriptorBufferInfo descriptor;
         descriptor.buffer = skin.ssbo->GetVkHandle();
         descriptor.range = sizeof(glm::mat4) * skin.inverseBindMatrices.size();;
         descriptor.offset = 0;

         skin.descriptorSet = std::make_shared<Vk::DescriptorSet>(device, mMeshSkinningDescriptorSetLayout.get(),
                                                                  mMeshSkinningDescriptorPool.get());
         skin.descriptorSet->BindStorageBuffer(0, &descriptor);
         skin.descriptorSet->UpdateDescriptorSets();
      }
   }

   void SkinAnimator::UpdateAnimation(float deltaTime)
   {
      if (mAnimations.size() == 0)
         return;

      if (mActiveAnimation > static_cast<uint32_t>(mAnimations.size()) - 1)
      {
         UTO_LOG("No animation with index " + std::to_string(mActiveAnimation));
         return;
      }

      Animation &animation = mAnimations[mActiveAnimation];
      animation.currentTime += deltaTime;
      if (animation.currentTime > animation.end)
      {
         animation.currentTime -= animation.end;
      }

      for (auto &channel : animation.channels)
      {
         AnimationSampler &sampler = animation.samplers[channel.samplerIndex];
         for (size_t i = 0; i < sampler.inputs.size() - 1; i++)
         {
            if (sampler.interpolation != "LINEAR")
            {
               UTO_LOG("This sample only supports linear interpolations");
               continue;
            }

            // Get the input keyframe values for the current time stamp
            if ((animation.currentTime >= sampler.inputs[i]) && (animation.currentTime <= sampler.inputs[i + 1]))
            {
               float a = (animation.currentTime - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
               if (channel.path == "translation")
               {
                  channel.node->translation = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], a);
               }
               if (channel.path == "rotation")
               {
                  glm::quat q1;
                  q1.x = sampler.outputsVec4[i].x;
                  q1.y = sampler.outputsVec4[i].y;
                  q1.z = sampler.outputsVec4[i].z;
                  q1.w = sampler.outputsVec4[i].w;

                  glm::quat q2;
                  q2.x = sampler.outputsVec4[i + 1].x;
                  q2.y = sampler.outputsVec4[i + 1].y;
                  q2.z = sampler.outputsVec4[i + 1].z;
                  q2.w = sampler.outputsVec4[i + 1].w;

                  channel.node->rotation = glm::normalize(glm::slerp(q1, q2, a));
               }
               if (channel.path == "scale")
               {
                  channel.node->scale = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], a);
               }
            }
         }
      }
   }

   void SkinAnimator::UpdateJoints(Node* node)
   {
      if (node->skin > -1)
      {
         // Update the joint matrices
         glm::mat4 inverseTransform = glm::inverse(GetNodeMatrix(node));
         Skin skin = mSkins[node->skin];
         size_t numJoints = (uint32_t) skin.joints.size();
         std::vector<glm::mat4> jointMatrices(numJoints);
         for (size_t i = 0; i < numJoints; i++)
         {
            jointMatrices[i] = GetNodeMatrix(skin.joints[i]) * skin.inverseBindMatrices[i];
            jointMatrices[i] = inverseTransform * jointMatrices[i];
         }

         skin.ssbo->UpdateMemory(jointMatrices.data(), jointMatrices.size() * sizeof(glm::mat4));
      }

      for (auto& child : node->children)
      {
         UpdateJoints(child);
      }
   }

   VkDescriptorSet SkinAnimator::GetJointMatricesDescriptorSet(int32_t skin)
   {
      assert(skin < mSkins.size());

      return mSkins[skin].descriptorSet->GetVkHandle();
   }

   glm::mat4 SkinAnimator::GetNodeMatrix(Node* node)
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
}
