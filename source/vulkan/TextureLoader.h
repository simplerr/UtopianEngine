#pragma once

#include <map>
#include <string>
#include "vulkan/VulkanInclude.h"
#include "utility/Module.h"
#include "utility/Common.h"

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
		SharedPtr<Texture> LoadTexture(std::string path);

		/** \brief Loads a cubemap texture from a file
		 *
		 * Stores the texture inside a std::map. Multiple loadings from the same
		 * file will return a pointer to the same Vulkan::Texture in the std::map. 
		 *
		 * \note TextureLoader handles the memory deallocation of the texture.
		 */
		SharedPtr<Texture> LoadCubemapTexture(std::string path);

		/** \brief Creates a texture from arbitrary data
		 *
		 * \note TextureLoader does NOT handle the memory deallocation of the texture.
		 */
		SharedPtr<Texture> CreateTexture(void* data, VkFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t pixelSize, VkImageAspectFlagBits aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, std::string name = "Unnamed Image");
	private:
		SharedPtr<Texture> LoadTextureGLI(std::string path);
		SharedPtr<Texture> LoadTextureSTB(std::string path);
	private:
		std::map<std::string, SharedPtr<Texture>> mTextureMap;
		Device*  mDevice;
		VkQueue mQueue;
	};

	TextureLoader& gTextureLoader();
}
