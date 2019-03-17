#pragma once

#include <vector>
#include "vulkan/VulkanInclude.h"

namespace Utopian::Vk
{
	/*
		Helper class when using arrays of combined image samplers in shaders.	
		Important that the number of elements matches the size of the array in the shader.
		Note: GLSL will optimize away unsedd variables which can affect the array size.
	*/
	class TextureArray
	{
	public:
		TextureArray();

		void AddTexture(VkImageView imageView, Sampler* sampler);
		void AddTexture(VkImageView imageView, VkSampler sampler);

		VkDescriptorImageInfo* GetImageInfo();
		uint32_t GetNumImages();
	private:
		std::vector<VkDescriptorImageInfo> mImageInfos;
	};

	class Texture
	{
	public:
		Texture(Device* device);
		~Texture();

		void CreateDescriptorSet(Device* device, DescriptorSetLayout* setLayout, DescriptorPool* descriptorPool);
		VkDescriptorImageInfo* GetTextureDescriptorInfo();

		void SetPath(std::string path);
		std::string GetPath();

		VkImage image;
		VkDeviceMemory deviceMemory;
		VkImageView imageView;
		VkSampler sampler;
	private:
		Device* mDevice;
		VkDescriptorImageInfo texDescriptor;
		std::string mPath;
	};
}
