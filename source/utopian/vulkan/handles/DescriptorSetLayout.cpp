#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/Debug.h"

namespace Utopian::Vk
{
   DescriptorSetLayout::DescriptorSetLayout(Device* device)
      : Handle(device, vkDestroyDescriptorSetLayout)
   {

   }

   void DescriptorSetLayout::AddBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags)
   {
      // Check for duplicates. Can happen if two different stages include a shared .glsl file.
      bool duplicate = false;
      for (uint32_t i = 0; i < mLayoutBindings.size(); i++)
      {
         if (mLayoutBindings[i].binding == binding)
         {
            assert(mLayoutBindings[i].descriptorType == descriptorType);
            assert(mLayoutBindings[i].stageFlags == stageFlags);
            duplicate = true;
         }
      }

      if (!duplicate)
      {
         VkDescriptorSetLayoutBinding layoutBinding = {};
         layoutBinding.binding = binding;
         layoutBinding.descriptorType = descriptorType;
         layoutBinding.descriptorCount = descriptorCount;
         layoutBinding.stageFlags = stageFlags;

         mLayoutBindings.push_back(layoutBinding);
      }
   }

   void DescriptorSetLayout::AddUniformBuffer(uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
   {
      AddBinding(binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorCount, stageFlags);
   }

   void DescriptorSetLayout::AddStorageBuffer(uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
   {
      AddBinding(binding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptorCount, stageFlags);
   }

   void DescriptorSetLayout::AddCombinedImageSampler(uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
   {
      AddBinding(binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorCount, stageFlags);
   }

   void DescriptorSetLayout::Create()
   {
      VkDescriptorSetLayoutCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      createInfo.bindingCount = (uint32_t)mLayoutBindings.size();
      createInfo.pBindings = mLayoutBindings.data();

      Debug::ErrorCheck(vkCreateDescriptorSetLayout(GetVkDevice(), &createInfo, nullptr, &mHandle));
   }
}