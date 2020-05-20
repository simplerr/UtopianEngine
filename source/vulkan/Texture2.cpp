#include "vulkan/Texture2.h"

namespace Utopian::Vk
{
	void Texture2::UpdateDescriptor()
	{
		descriptor.sampler = sampler->GetVkHandle();
		descriptor.imageView = image->GetView();
		descriptor.imageLayout = image->GetFinalLayout();
	}

	Texture::Texture()
	{

	}

	Texture::Texture(const TEXTURE_CREATE_INFO& createInfo, Device* device)
	{
		LoadFromFile(createInfo.path, createInfo.format, device, createInfo.imageUsageFlags, createInfo.imageLayout);
	}

	Texture::Texture(std::string filename, Device* device)
	{
		TEXTURE_CREATE_INFO createInfo;
		createInfo.path = filename;

		LoadFromFile(createInfo.path, createInfo.format, device, createInfo.imageUsageFlags, createInfo.imageLayout);
	}

	void Texture::LoadFromFile(std::string filename,
								 VkFormat format,
								 Device* device,
								 VkImageUsageFlags imageUsageFlags,
								 VkImageLayout imageLayout)
	{
		gli::Texture tex2D(gli::load(filename.c_str()));

		assert(!tex2D.empty());

		mWidth = static_cast<uint32_t>(tex2D[0].extent().x);
		mHeight = static_cast<uint32_t>(tex2D[0].extent().y);
		mNumMipLevels = static_cast<uint32_t>(tex2D.levels());

		BUFFER_CREATE_INFO bufferDesc;
		bufferDesc.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferDesc.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		bufferDesc.data = tex2D.data();
		bufferDesc.size = tex2D.size();
		Buffer stagingBuffer = Buffer(bufferDesc, device);

		IMAGE_CREATE_INFO imageDesc;
		imageDesc.width = mWidth;
		imageDesc.height = mHeight;
		imageDesc.format = format;
		imageDesc.usage = imageUsageFlags | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageDesc.mipLevels = mNumMipLevels;
		image = std::make_shared<Vk::Image>(imageDesc, device);

		// Setup buffer copy regions for each mip level
		std::vector<VkBufferImageCopy> copyRegions;
		uint32_t offset = 0;

		for (uint32_t i = 0; i < mNumMipLevels; i++)
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

			copyRegions.push_back(bufferCopyRegion);
			offset += static_cast<uint32_t>(tex2D[i].size());
		}

		CommandBuffer cmdBuffer = CommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		// Transfer image to a valid layout
		image->LayoutTransition(device, cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		stagingBuffer.Copy(&cmdBuffer, image.get(), copyRegions);

		// Transfer back to final layouts
		image->LayoutTransition(device, cmdBuffer, imageLayout);

		cmdBuffer.Flush();

		// Create a default sampler
		sampler = std::make_shared<Vk::Sampler>(device, false);
		sampler->createInfo.minLod = 0.0f;
		sampler->createInfo.maxLod = (float)mNumMipLevels;
		sampler->createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		sampler->Create();

		UpdateDescriptor();
	}

	
}