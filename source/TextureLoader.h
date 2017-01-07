#pragma once
#include <string>
#include <vulkan/vulkan.h>

namespace VulkanLib
{
	class VulkanDevice;

	struct VulkanTexture
	{
		VkImage image;
		VkDeviceMemory deviceMemory;
	};

	class TextureLoader
	{
	public:
		TextureLoader(VulkanDevice* vulkanDevice, VkQueue queue);
		~TextureLoader();

		bool LoadTexture(std::string filename, VulkanTexture* texture);

		void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);
	private:
		VulkanDevice* mVulkanDevice;
		VkQueue mQueue;
	};
}
