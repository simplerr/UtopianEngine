#pragma once

#include <vulkan/vulkan.h>

namespace Vulkan
{
	class Device;
	class DescriptorSet;
	class DescriptorPool;
	class DescriptorSetLayout;

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
