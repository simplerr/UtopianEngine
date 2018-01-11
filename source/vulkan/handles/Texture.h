#pragma once

#include <vulkan/vulkan.h>
#include "vulkan/VulkanInclude.h"

namespace Vulkan
{
	class Texture
	{
	public:
		Texture(Device* device);
		~Texture();

		void CreateDescriptorSet(Device* device, DescriptorSetLayout* setLayout, DescriptorPool* descriptorPool);
		DescriptorSet* GetDescriptorSet();
		VkDescriptorImageInfo GetTextureDescriptorInfo();

		VkImage image;
		VkDeviceMemory deviceMemory;
		VkImageView imageView;
		VkSampler sampler;
	private:
		Device* mDevice;
		DescriptorSet* mDescriptorSet;
	};
}
