#include "Sampler.h"
#include "vulkan/Device.h"
#include "vulkan/VulkanDebug.h"

namespace Utopian::Vk
{
	Sampler::Sampler(Device* device, bool create)
		: Handle(device, vkDestroySampler)
	{
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;	// VK_COMPARE_OP_NEVER in Sascha Willems code
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		if (create)
		{
			Create();
		}
	}

	void Sampler::Create()
	{
		VulkanDebug::ErrorCheck(vkCreateSampler(GetDevice(), &samplerInfo, nullptr, &mHandle));
	}
}