#pragma once

#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"
#include <vector>
#include <map>
#include "vulkan/handles/DescriptorSetLayout.h"

namespace Utopian::Vk
{
   /**
    * Wraps both the handling of pipeline layouts and descriptor set layouts.
    * As a pipeline layout is descriptor set layouts + push constant ranges
    * it makes sense to group them together as they form the "interface" to the 
    * shaders in the pipeline.
    */
   class PipelineInterface
   {
   public:
      PipelineInterface(Device* device);
      ~PipelineInterface();

      /**
       * Creates the descriptor set layout and pipeline layout objects  
       * @note Must be called after all different descriptor types
       * and push constant ranges have been added.
       */
      void Create();

      /** Adds different descriptor types to the corresponding descriptor set layout. */
      void AddUniformBuffer(uint32_t descriptorSet, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);
      void AddStorageBuffer(uint32_t descriptorSet, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);
      void AddCombinedImageSampler(uint32_t descriptorSet, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);
      void AddStorageImage(uint32_t descriptorSet, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);

      /** Adds a push constant range to the pipeline layout. */
      void AddPushConstantRange(uint32_t size, VkShaderStageFlags shaderStage, uint32_t offset = 0);

      DescriptorSetLayout* GetDescriptorSetLayout(uint32_t descriptorSet);
      VkDescriptorSetLayout GetVkDescriptorSetLayout(uint32_t descriptorSet);
      VkPipelineLayout GetPipelineLayout() const;
      uint32_t GetNumDescriptorSets() const;

   private:
      void AddDescriptor(VkDescriptorType descriptorType, uint32_t descriptorSet, uint32_t binding, uint32_t descriptorCount, VkShaderStageFlags stageFlags);

      Device* mDevice;
      VkPipelineLayout mPipelineLayout;
      std::map<uint32_t, SharedPtr<DescriptorSetLayout>> mDescriptorSetLayouts;
      std::vector<VkPushConstantRange> mPushConstantRanges;
   };
}
