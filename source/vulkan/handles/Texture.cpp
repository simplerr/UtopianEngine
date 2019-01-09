#include "vulkan/handles/Texture.h"
#include "vulkan/handles/Sampler.h"
#include "vulkan/handles/DescriptorSet.h"

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
		texDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		return &texDescriptor;
	}
}
