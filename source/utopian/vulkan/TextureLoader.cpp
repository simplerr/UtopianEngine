#include "vulkan/VulkanApp.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/Texture.h"
#include "vulkan/handles/Queue.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/Sampler.h"
#include "TextureLoader.h"
#include "Debug.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/Image.h"
#include "utility/Utility.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image.h"
#include <gli/gli.hpp>
#include <gli/type.hpp>

namespace Utopian::Vk
{
	TextureLoader::TextureLoader(Device* device)
	{
		mDevice = device;
		mQueue = mDevice->GetQueue()->GetVkHandle();
	}

	TextureLoader::~TextureLoader()
	{
	}

	TextureLoader& gTextureLoader()
	{
		return TextureLoader::Instance();
	}

	SharedPtr<Texture> TextureLoader::LoadTexture(std::string path, VkFormat format)
	{
		// Check if the model is loaded already
		if (mTextureMap.find(path) != mTextureMap.end())
			return mTextureMap[path];

		std::string extension = GetFileExtension(path);

		SharedPtr<Texture> texture;
		if (extension == ".ktx" || extension == ".dds")
		{
			texture = LoadTextureGLI(path, format);
		}
		else
		{
			texture = LoadTextureSTB(path);
		}

		mTextureMap[path] = texture;
		texture->SetPath(path);

		if (texture != nullptr)
			texture->UpdateDescriptor();

		return texture;
	}
	
	SharedPtr<Texture> TextureLoader::LoadTextureGLI(std::string path, VkFormat format)
	{
		// These might be needed as parameters
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;

		gli::texture2d tex2D(gli::load(path.c_str()));

		assert(!tex2D.empty());

		uint32_t width = static_cast<uint32_t>(tex2D[0].extent().x);
		uint32_t height = static_cast<uint32_t>(tex2D[0].extent().y);
		uint32_t numMipLevels = static_cast<uint32_t>(tex2D.levels());

		BUFFER_CREATE_INFO bufferDesc;
		bufferDesc.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferDesc.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		bufferDesc.data = tex2D.data();
		bufferDesc.size = tex2D.size();
		bufferDesc.name = "Texture loader GLI staging buffer";
		Buffer stagingBuffer = Buffer(bufferDesc, mDevice);

		IMAGE_CREATE_INFO imageDesc;
		imageDesc.width = width;
		imageDesc.height = height;
		imageDesc.format = format;
		imageDesc.usage = imageUsageFlags | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		imageDesc.mipLevels = numMipLevels;
		imageDesc.name = "Texture: "+ path;
		SharedPtr<Image> image = std::make_shared<Vk::Image>(imageDesc, mDevice);

		// Setup buffer copy regions for each mip level
		std::vector<VkBufferImageCopy> bufferCopyRegions;
		uint32_t offset = 0;

		for (uint32_t i = 0; i < numMipLevels; i++)
		{
			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = i;
			bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(tex2D[i].extent().x);
			bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(tex2D[i].extent().y);;
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = offset;

			bufferCopyRegions.push_back(bufferCopyRegion);
			offset += static_cast<uint32_t>(tex2D[i].size());
		}

		CommandBuffer cmdBuffer = CommandBuffer(mDevice, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		// Transfer image to a valid layout
		image->LayoutTransition(cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		stagingBuffer.Copy(&cmdBuffer, image.get(), bufferCopyRegions);

		// Transfer back to final layouts
		image->LayoutTransition(cmdBuffer, imageLayout);

		cmdBuffer.Flush();

		// Create the sampler
		SharedPtr<Sampler> sampler = std::make_shared<Vk::Sampler>(mDevice, false);
		sampler->createInfo.minLod = 0.0f;
		sampler->createInfo.maxLod = (float)numMipLevels;
		sampler->createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		sampler->Create();

		SharedPtr<Texture> texture = std::make_shared<Texture>(mDevice);
		texture->mImage = image;
		texture->mSampler = sampler;
		texture->mWidth = width;
		texture->mHeight = height;

		return texture;
	}

	SharedPtr<Texture> TextureLoader::LoadTextureSTB(std::string path)
	{
		int width, height, texChannels;
		uint32_t pixelSize = sizeof(uint32_t);
		stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = width * height * pixelSize;

		if (!pixels) {
			return nullptr;
		}

		// Temporary image used to retrieve the subresource layout needed for correct copying of image data
		IMAGE_CREATE_INFO imageDesc;
		imageDesc.width = width;
		imageDesc.height = height;
		imageDesc.tiling = VK_IMAGE_TILING_LINEAR;
		imageDesc.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		imageDesc.name = "Texture loader temporary image";
		Image temporaryImage = Image(imageDesc, mDevice);

		VkSubresourceLayout imageLayout = temporaryImage.GetSubresourceLayout(mDevice);

		uint32_t* data = new uint32_t[imageSize];

		if (imageLayout.rowPitch == width * pixelSize) {
			memcpy(data, pixels, (size_t)imageSize);
		}
		else {
			uint8_t* dataBytes = reinterpret_cast<uint8_t*>(data);

			for (int y = 0; y < height; y++) {
				memcpy(&dataBytes[y * imageLayout.rowPitch], &pixels[y * width * pixelSize], height * pixelSize);
			}
		}

		SharedPtr<Texture> texture = CreateTexture(data, VK_FORMAT_R8G8B8A8_UNORM,
												   width, height, 1, pixelSize,
												   VK_IMAGE_ASPECT_COLOR_BIT, "Texture: " + path);

		stbi_image_free(pixels);
		delete[] data;

		return texture;
	}

	SharedPtr<Texture> TextureLoader::CreateTexture(void* data, VkFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t pixelSize, VkImageAspectFlagBits aspectMask, std::string name)
	{
		VkDevice device = mDevice->GetVkDevice();
		VkDeviceSize imageSize = width * height * depth * pixelSize; // NOTE: Assumes each pixel is stored as U8

		// Create staging image and fill it with data
		IMAGE_CREATE_INFO imageDesc;
		imageDesc.width = width;
		imageDesc.height = height;
		imageDesc.depth = depth;
		imageDesc.format = format;
		imageDesc.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageDesc.tiling = VK_IMAGE_TILING_LINEAR;
		imageDesc.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		imageDesc.name = "Texture loader staging texture";
	 	Image stagingImage = Image(imageDesc, mDevice);

		if (data != nullptr)
			stagingImage.UpdateMemory(data, imageSize);

		// Create the final image
		imageDesc.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageDesc.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageDesc.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		imageDesc.name = name;
		SharedPtr<Image> image = std::make_shared<Vk::Image>(imageDesc, mDevice);

		CommandBuffer cmdBuffer(mDevice, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		// Prepare images for copying
		stagingImage.LayoutTransition(cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		image->LayoutTransition(cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		stagingImage.Copy(cmdBuffer, image.get());

		// Transition to final layout
		image->LayoutTransition(cmdBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		cmdBuffer.Flush();

		// Create the sampler
		SharedPtr<Sampler> sampler = std::make_shared<Vk::Sampler>(mDevice, false);
		sampler->createInfo.minLod = 0.0f;
		sampler->createInfo.maxLod = 1.0f;
		sampler->createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		sampler->Create();

		SharedPtr<Texture> texture = std::make_shared<Texture>(mDevice);
		texture->mImage = image;
		texture->mSampler = sampler;
		texture->mWidth = width;
		texture->mHeight = height;
		texture->UpdateDescriptor();

		return texture;
	}

	SharedPtr<Texture> TextureLoader::LoadCubemapTexture(std::string path)
	{
		// These might be needed as parameters
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

		gli::texture_cube texCube(gli::load(path));

		assert(!texCube.empty());

		uint32_t width = static_cast<uint32_t>(texCube.extent().x);
		uint32_t height = static_cast<uint32_t>(texCube.extent().y);
		uint32_t numMipLevels = static_cast<uint32_t>(texCube.levels());

		BUFFER_CREATE_INFO bufferDesc;
		bufferDesc.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferDesc.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		bufferDesc.data = texCube.data();
		bufferDesc.size = texCube.size();
		Buffer stagingBuffer = Buffer(bufferDesc, mDevice);

		IMAGE_CREATE_INFO imageDesc;
		imageDesc.width = width;
		imageDesc.height = height;
		imageDesc.format = format;
		imageDesc.usage = imageUsageFlags | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageDesc.mipLevels = numMipLevels;
		imageDesc.arrayLayers = 6; // Cube faces count as array layers in Vulkan
		imageDesc.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT; // This flag is required for cube map images
		SharedPtr<Image> image = std::make_shared<Vk::Image>(imageDesc, mDevice);

		// Setup buffer copy regions for each face including all of it's miplevels
		std::vector<VkBufferImageCopy> bufferCopyRegions;
		size_t offset = 0;

		for (uint32_t face = 0; face < 6; face++)
		{
			for (uint32_t level = 0; level < numMipLevels; level++)
			{
				VkBufferImageCopy bufferCopyRegion = {};
				bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				bufferCopyRegion.imageSubresource.mipLevel = level;
				bufferCopyRegion.imageSubresource.baseArrayLayer = face;
				bufferCopyRegion.imageSubresource.layerCount = 1;
				bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(texCube[face][level].extent().x);
				bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(texCube[face][level].extent().y);
				bufferCopyRegion.imageExtent.depth = 1;
				bufferCopyRegion.bufferOffset = offset;

				bufferCopyRegions.push_back(bufferCopyRegion);

				// Increase offset into staging buffer for next level / face
				offset += texCube[face][level].size();
			}
		}

		CommandBuffer cmdBuffer = CommandBuffer(mDevice, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		// Transfer image to a valid layout
		image->LayoutTransition(cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		stagingBuffer.Copy(&cmdBuffer, image.get(), bufferCopyRegions);

		// Transfer back to final layouts
		image->LayoutTransition(cmdBuffer, imageLayout);

		cmdBuffer.Flush();

		// Create the sampler
		SharedPtr<Sampler> sampler = std::make_shared<Vk::Sampler>(mDevice, false);
		sampler->createInfo.minLod = 0.0f;
		sampler->createInfo.maxLod = (float)numMipLevels;
		sampler->createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		sampler->Create();

		SharedPtr<Texture> texture = std::make_shared<Texture>(mDevice);
		texture->mImage = image;
		texture->mSampler = sampler;
		texture->mWidth = width;
		texture->mHeight = height;
		texture->UpdateDescriptor();

		return texture;
	}
}
