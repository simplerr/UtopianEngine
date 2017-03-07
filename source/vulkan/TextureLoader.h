#pragma once

#include <map>
#include <string>
#include <vulkan/vulkan.h>

#define PLACEHOLDER_TEXTURE_PATH "data/textures/checker.jpg"

namespace Vulkan
{
	class Device;
	class DescriptorSet;
	class DescriptorSetLayout;
	class DescriptorPool;
	class Renderer;
	class Texture;

	class TextureLoader
	{
	public:
		TextureLoader(Renderer* renderer, VkQueue queue);
		~TextureLoader();

		Texture* LoadTexture(std::string filename);

		// NOTE: Textures loaded with this function needs to be deleted manually
		Texture* LoadTexture(DescriptorSetLayout* layout, DescriptorPool* pool, void* data, VkFormat format, uint32_t width, uint32_t height, uint32_t pixelSize);

		void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);
		void CreateImageView(VkImage image, VkFormat format, VkImageView* imageView);
		void CreateImageSampler(VkSampler* sampler);
	private:
		std::map<std::string, Texture*> mTextureMap;
		Renderer* mRenderer;
		Device*  mDevice;
		VkQueue mQueue;
	};
}
