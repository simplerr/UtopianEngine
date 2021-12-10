#pragma once

#include <vector>
#include "Handle.h"
#include "vulkan/VulkanPrerequisites.h"

namespace Utopian::Vk
{
   /**
    * Wrapper for handling pipeline layouts.
    * @note This is currently not used by PipelineInterface.
    */
   class PipelineLayout : public Handle<VkPipelineLayout>
   {
   public:
      PipelineLayout(Device* device);

      void AddDescriptorSetLayout(DescriptorSetLayout* descriptorSetLayout);
      void AddPushConstantRange(VkShaderStageFlags shaderStage, uint32_t size, uint32_t offset = 0);
      void Create();

   private:
      std::vector<VkDescriptorSetLayout> mDescriptorSetLayouts;
      VkPushConstantRange mPushConstantRange = {};
      bool mPushConstantActive = false;
   };
}
