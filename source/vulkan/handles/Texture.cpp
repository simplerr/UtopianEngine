#include "vulkan/handles/Texture.h"
#include "vulkan/handles/Sampler.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/Texture2.h"

namespace Utopian::Vk
{
	

	Texture::Texture(Device* device)
	{
		mDevice = device;
	}

	Texture::~Texture()
	{
	}

	VkDescriptorImageInfo* Texture::GetTextureDescriptorInfo()
	{
		texDescriptor = {};
		texDescriptor.sampler = mSampler->GetVkHandle();
		texDescriptor.imageView = mImage->GetView();
		texDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		return &texDescriptor;
	}

	Image* Texture::GetImage()
	{
		return mImage.get();
	}
	
	Sampler* Texture::GetSampler()
	{
		return mSampler.get();
	}

	void Texture::SetPath(std::string path)
	{
		mPath = path;
	}

	std::string Texture::GetPath()
	{
		return mPath;
	}

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

	void TextureArray::AddTexture(const SharedPtr<Vk::Texture>& texture)
	{
		VkDescriptorImageInfo imageInfo;
		imageInfo.sampler = texture->GetSampler()->GetVkHandle();
		imageInfo.imageView = texture->GetImage()->GetView();
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
}
