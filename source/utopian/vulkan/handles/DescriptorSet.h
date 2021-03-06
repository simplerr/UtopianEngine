#pragma once

#include <vector>
#include <map>
#include "vulkan/handles/Handle.h"
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"

namespace Utopian::Vk
{
   /**
    * Wrapper for handling descriptor sets, making binding descriptors to them easier.
    */
   class DescriptorSet
   {
   public:
      DescriptorSet(Device* device, DescriptorSetLayout* setLayout, DescriptorPool* descriptorPool);
      DescriptorSet(Device* device, Effect* effect, uint32_t set, DescriptorPool* descriptorPool);
      ~DescriptorSet();

      /** 
       * Update the descriptors bound to this descriptor set.
       * @note Needs to be called after calling the Bind* functions for them to have effect. 
       */
      void UpdateDescriptorSets();

      /** Functions for binding different types of descriptors by ID. */
      void BindUniformBuffer(uint32_t binding, const VkDescriptorBufferInfo* bufferInfo);
      void BindStorageBuffer(uint32_t binding, const VkDescriptorBufferInfo* bufferInfo);
      void BindCombinedImage(uint32_t binding, const VkDescriptorImageInfo* imageInfo, uint32_t descriptorCount = 1);
      void BindCombinedImage(uint32_t binding, const Image& image, const Sampler& sampler);
      void BindCombinedImage(uint32_t binding, VkImageView imageView, VkSampler sampler, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      void BindImage(uint32_t binding, VkImageView imageView, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      void BindImage(uint32_t binding, const Image& image);

      /**
       * Functions for binding different types of descriptors by their name. 
       * @note the name argument must match the name in the GLSL shader.
       */
      void BindUniformBuffer(std::string name, const VkDescriptorBufferInfo* bufferInfo);
      void BindStorageBuffer(std::string name, const VkDescriptorBufferInfo* bufferInfo);
      void BindCombinedImage(std::string name, const VkDescriptorImageInfo* imageInfo, uint32_t descriptorCount = 1);
      void BindCombinedImage(std::string name, const Image& image, const Sampler& sampler);
      void BindCombinedImage(std::string name, VkImageView imageView, VkSampler sampler);
      void BindImage(std::string name, const Image& image);

      VkDescriptorSet GetVkHandle() const;

      void SetShader(Shader* shader);

   private:
      void Create(Device* device, DescriptorSetLayout* setLayout, DescriptorPool* descriptorPool);

   private:
      std::vector<VkWriteDescriptorSet> mWriteDescriptorSets;
      std::map<int, VkDescriptorImageInfo> mImageInfoMap;
      VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;
      DescriptorSetLayout* mSetLayout;
      Device* mDevice;

      /**
       * The shader that this descriptor set was created from.
       * This is used in order to get access to the shader reflection
       * to perform the string -> binding lookup.
       * Note: Maybe don't belong here. */
      Shader* mShader = nullptr;
   };

   /** Wrapper for VkDescriptorPool. */
   class DescriptorPool : public Handle<VkDescriptorPool>
   {
   public:
      DescriptorPool(Device* device);
      void AddDescriptor(VkDescriptorType type, uint32_t count);
      void Create();
   private:
      std::vector<VkDescriptorPoolSize> mDescriptorSizes;
   };
}
