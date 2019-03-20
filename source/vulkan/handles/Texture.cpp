#include "vulkan/handles/Texture.h"
#include "vulkan/handles/Sampler.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/Texture2.h"

namespace Utopian::Vk
{
	TextureArray::TextureArray()
	{

	}

	void TextureArray::AddTexture(VkImageView imageView, Sampler* sampler)
	{
		VkDescriptorImageInfo imageInfo;
		imageInfo.sampler = sampler->GetVkHandle();
		imageInfo.imageView = imageView;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		mImageInfos.push_back(imageInfo);
	}

	void TextureArray::AddTexture(VkImageView imageView, VkSampler sampler)
	{
		VkDescriptorImageInfo imageInfo;
		imageInfo.sampler = sampler;
		imageInfo.imageView = imageView;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		mImageInfos.push_back(imageInfo);
	}

	void TextureArray::AddTexture(const SharedPtr<Vk::Texture2D>& texture)
	{
		VkDescriptorImageInfo imageInfo;
		imageInfo.sampler = texture->sampler;
		imageInfo.imageView = texture->view;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		mImageInfos.push_back(imageInfo);
	}

	VkDescriptorImageInfo* TextureArray::GetImageInfo()
	{
		return mImageInfos.data();
	}
	
	uint32_t TextureArray::GetNumImages()
	{
		return mImageInfos.size();
	}

	Texture::Texture(Device* device)
	{
		mDevice = device;
	}

	Texture::~Texture()
	{
		vkDestroyImageView(mDevice->GetVkDevice(), imageView, nullptr);
		vkDestroyImage(mDevice->GetVkDevice(), image, nullptr);
		vkDestroySampler(mDevice->GetVkDevice(), sampler, nullptr);
		vkFreeMemory(mDevice->GetVkDevice(), deviceMemory, nullptr);
	}

	VkDescriptorImageInfo* Texture::GetTextureDescriptorInfo()
	{
		texDescriptor = {};
		texDescriptor.sampler = sampler;
		texDescriptor.imageView = imageView;
		texDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		return &texDescriptor;
	}

	void Texture::SetPath(std::string path)
	{
		mPath = path;
	}

	std::string Texture::GetPath()
	{
		return mPath;
	}
}
