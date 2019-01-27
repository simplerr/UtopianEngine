#pragma once

#include <map>
#include <string>
#include "vulkan/VulkanInclude.h"
#include "utility/Module.h"

#define PLACEHOLDER_TEXTURE_PATH "data/textures/WoodDetail1A_D.png"

namespace Utopian::Vk
{
	class TextureLoader : public Module<TextureLoader>
	{
	public:
		TextureLoader(Device* device);
		~TextureLoader();

		/** \brief Loads a texture from a file
		 *
		 * Stores the texture inside a std::map. Multiple loadings from the same
		 * file will return a pointer to the same Vulkan::Texture in the std::map. 
		 *
		 * \note TextureLoader handles the memory deallocation of the texture.
		 */
		Texture* LoadTexture(std::string filename);
		Texture* LoadCubeMap(std::string filename);

		/** \brief Creates a texture from arbitrary data
		 *
		 * \note TextureLoader does NOT handle the memory deallocation of the texture.
		 */
		Texture* CreateTexture(void* data, VkFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t pixelSize, VkImageAspectFlagBits aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);

	private:
		void CreateImage(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlagBits aspectMask);
		void CopyImage(VkImage srcImage, VkImage dstImage, VkImageAspectFlagBits aspectMask, uint32_t width, uint32_t height, uint32_t depth);
		void CreateImageView(VkImage image, VkFormat format, VkImageView* imageView, uint32_t depth, VkImageAspectFlagBits aspectMask);
		void CreateImageSampler(VkSampler* sampler);

	private:
		std::map<std::string, Texture*> mTextureMap;
		Device*  mDevice;
		VkQueue mQueue;
	};

	TextureLoader& gTextureLoader();
}
