#include "vulkan/handles/Texture.h"
#include "vulkan/handles/DescriptorSet.h"

namespace Vulkan
{
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

	void Texture::CreateDescriptorSet(Device* device, DescriptorSetLayout* setLayout, DescriptorPool* descriptorPool)
	{
		mDescriptorSet = new DescriptorSet(device, setLayout, descriptorPool);
		mDescriptorSet->BindCombinedImage(0, &GetTextureDescriptorInfo());	// NOTE: It's hard to know that the texture must be bound to binding=0
		mDescriptorSet->UpdateDescriptorSets();
	}

	DescriptorSet* Texture::GetDescriptorSet()
	{
		return mDescriptorSet;
	}

	VkDescriptorImageInfo Texture::GetTextureDescriptorInfo()
	{
		VkDescriptorImageInfo texDescriptor = {};
		texDescriptor.sampler = sampler;
		texDescriptor.imageView = imageView;
		texDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		return texDescriptor;
	}
}
