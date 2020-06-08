#pragma once

#include <vector>
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"

namespace Utopian::Vk
{
	/*
	 * Represents a texture object.
	 * Textures are created from TextureLoader.
	 */
	class Texture
	{
	public:
		Texture(Device* device);
		~Texture();

		void UpdateDescriptor();

		Image* GetImage();
		Sampler* GetSampler();
		std::string GetPath() const;
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		uint32_t GetNumMipLevels() const;

		const VkDescriptorImageInfo* GetDescriptor() const;

		void SetPath(std::string path);

	private:
		SharedPtr<Vk::Image> mImage;
		SharedPtr<Vk::Sampler> mSampler;
		VkDescriptorImageInfo mDescriptor;
		std::string mPath;
		uint32_t mWidth;
		uint32_t mHeight;
		uint32_t mNumMipLevels;
		Device* mDevice;

		friend class TextureLoader;
	};

	/*
	 *	Helper class when using arrays of combined image samplers in shaders.	
	 *	Important that the number of elements matches the size of the array in the shader.
     *  Note: GLSL will optimize away unused variables which can affect the array size.
	 */
	class TextureArray
	{
	public:
		TextureArray();

		void AddTexture(const SharedPtr<Vk::Texture>& texture);

		const VkDescriptorImageInfo* GetDescriptor() const;
		uint32_t GetNumImages() const;
	private:
		std::vector<VkDescriptorImageInfo> mImageInfos;
	};
}
