#pragma once

#include <vector>
#include "vulkan/VulkanInclude.h"
#include "utility/Common.h"

namespace Utopian::Vk
{
	class Texture
	{
	public:
		Texture(Device* device);
		~Texture();

		void CreateDescriptorSet(Device* device, DescriptorSetLayout* setLayout, DescriptorPool* descriptorPool);
		VkDescriptorImageInfo* GetTextureDescriptorInfo();

		Image* GetImage();
		Sampler* GetSampler();

		void SetPath(std::string path);
		std::string GetPath();

	private:
		SharedPtr<Vk::Image> mImage;
		SharedPtr<Vk::Sampler> mSampler;
		Device* mDevice;
		VkDescriptorImageInfo texDescriptor;
		std::string mPath;

		friend class TextureLoader;
	};

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
		void AddTexture(const SharedPtr<Vk::Texture>& texture);

		VkDescriptorImageInfo* GetImageInfo();
		uint32_t GetNumImages();
	private:
		std::vector<VkDescriptorImageInfo> mImageInfos;
	};
}
