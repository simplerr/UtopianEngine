#pragma once

#include "vulkan/VulkanInclude.h"

namespace Utopian::Vk
{
	class Texture
	{
	public:
		Texture(Device* device);
		~Texture();

		void CreateDescriptorSet(Device* device, DescriptorSetLayout* setLayout, DescriptorPool* descriptorPool);
		DescriptorSet* GetDescriptorSet();
		VkDescriptorImageInfo* GetTextureDescriptorInfo();

		VkImage image;
		VkDeviceMemory deviceMemory;
		VkImageView imageView;
		VkSampler sampler;
	private:
		Device* mDevice;
		DescriptorSet* mDescriptorSet;
		VkDescriptorImageInfo texDescriptor;
	};
}
