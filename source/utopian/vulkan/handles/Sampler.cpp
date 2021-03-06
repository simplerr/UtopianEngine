#include "Sampler.h"
#include "vulkan/handles/Device.h"
#include "vulkan/Debug.h"

namespace Utopian::Vk
{
   Sampler::Sampler(Device* device, bool create)
      : Handle(device, vkDestroySampler)
   {
      createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
      createInfo.magFilter = VK_FILTER_LINEAR;
      createInfo.minFilter = VK_FILTER_LINEAR;
      createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      createInfo.anisotropyEnable = VK_TRUE;
      createInfo.maxAnisotropy = 16;
      createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
      createInfo.unnormalizedCoordinates = VK_FALSE;
      createInfo.compareEnable = VK_FALSE;
      createInfo.compareOp = VK_COMPARE_OP_ALWAYS; // VK_COMPARE_OP_NEVER in Sascha Willems code
      createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

      if (create)
      {
         Create();
      }
   }

   void Sampler::Create()
   {
      Debug::ErrorCheck(vkCreateSampler(GetVkDevice(), &createInfo, nullptr, &mHandle));
   }
}