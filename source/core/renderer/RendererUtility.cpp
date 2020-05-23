#include "core/renderer/RendererUtility.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Pipeline.h"
#include <fstream>

namespace Utopian
{
	RendererUtility& gRendererUtility()
	{
		return RendererUtility::Instance();
	}

	void RendererUtility::DrawFullscreenQuad(Vk::CommandBuffer* commandBuffer)
	{
		commandBuffer->CmdDraw(3, 1, 0, 0);
	}

	void RendererUtility::SetAdditiveBlending(Vk::Pipeline* pipeline)
	{
		// Enable additive blending
		pipeline->blendAttachmentState[0].blendEnable = VK_TRUE;
		pipeline->blendAttachmentState[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		pipeline->blendAttachmentState[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		pipeline->blendAttachmentState[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		pipeline->blendAttachmentState[0].colorBlendOp = VK_BLEND_OP_ADD;
		pipeline->blendAttachmentState[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		pipeline->blendAttachmentState[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
		pipeline->blendAttachmentState[0].alphaBlendOp = VK_BLEND_OP_ADD;
	}

	void RendererUtility::SetAlphaBlending(Vk::Pipeline* pipeline)
	{
		pipeline->blendAttachmentState[0].blendEnable = VK_TRUE;
		pipeline->blendAttachmentState[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		pipeline->blendAttachmentState[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		pipeline->blendAttachmentState[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipeline->blendAttachmentState[0].colorBlendOp = VK_BLEND_OP_ADD;
		pipeline->blendAttachmentState[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipeline->blendAttachmentState[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		pipeline->blendAttachmentState[0].alphaBlendOp = VK_BLEND_OP_ADD;
	}

	void RendererUtility::SaveToFile(Vk::Device* device, const SharedPtr<Vk::Image>& image, std::string filename, uint32_t width, uint32_t height)
	{
		SharedPtr<Vk::Image> hostVisibleImage = CreateHostVisibleImage(device, image, width, height, VK_FORMAT_R8G8B8A8_UNORM);

		// Get layout of the image (including row pitch)
		VkImageSubresource subResource{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
		VkSubresourceLayout subResourceLayout;
		vkGetImageSubresourceLayout(device->GetVkDevice(), hostVisibleImage->GetVkHandle(), &subResource, &subResourceLayout);

		const char* data;
		vkMapMemory(device->GetVkDevice(), hostVisibleImage->GetDeviceMemory(), 0, VK_WHOLE_SIZE, 0, (void**)&data);
		data += subResourceLayout.offset;

		std::ofstream file(filename, std::ios::out | std::ios::binary);

		// ppm header
		file << "P6\n" << width << "\n" << height << "\n" << 255 << "\n";

		// ppm binary pixel data
		for (uint32_t y = 0; y < height; y++)
		{
			unsigned int *row = (unsigned int*)data;
			for (uint32_t x = 0; x < width; x++)
			{
				file.write((char*)row, 3);
				row++;
			}
			data += subResourceLayout.rowPitch;
		}

		file.close();
	}

	SharedPtr<Vk::Image> RendererUtility::CreateHostVisibleImage(Vk::Device* device, const SharedPtr<Vk::Image>& srcImage, uint32_t width, uint32_t height, VkFormat format)
	{
		CopyImageInfo info;
		info.width = width;
		info.height = height;
		info.tiling = VK_IMAGE_TILING_LINEAR;
		info.format = format;
		info.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		info.finalImageLayout = VK_IMAGE_LAYOUT_GENERAL;
		info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		SharedPtr<Vk::Image> hostVisibleImage = gRendererUtility().CopyImage(device, srcImage, info);

		return hostVisibleImage;
	}

	SharedPtr<Vk::Image> RendererUtility::CopyImage(Vk::Device* device, const SharedPtr<Vk::Image>& srcImage, const CopyImageInfo& info)
	{
		Vk::IMAGE_CREATE_INFO createInfo;
		createInfo.width = info.width;
		createInfo.height = info.height;
		createInfo.format = info.format;
		createInfo.usage = info.usage;
		createInfo.properties = info.memoryProperties;
		createInfo.tiling = info.tiling;
		SharedPtr<Vk::Image> dstImage = std::make_shared<Vk::Image>(createInfo, device);
		dstImage->SetFinalLayout(info.finalImageLayout);

		VkFormatProperties formatProps;

		// Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
		vkGetPhysicalDeviceFormatProperties(device->GetPhysicalDevice(), srcImage->GetFormat(), &formatProps);
		if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
			assert(0);
		}

		// Check if the device supports blitting to linear images 
		vkGetPhysicalDeviceFormatProperties(device->GetPhysicalDevice(), dstImage->GetFormat(), &formatProps);
		if (!(formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
			assert(0);
		}

		Vk::CommandBuffer commandBuffer = Vk::CommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		dstImage->LayoutTransition(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		srcImage->LayoutTransition(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		srcImage->Blit(commandBuffer, dstImage.get());

		dstImage->LayoutTransition(commandBuffer, dstImage->GetFinalLayout());
		srcImage->LayoutTransition(commandBuffer, srcImage->GetFinalLayout());

		commandBuffer.Flush();

		return dstImage;
	}
}