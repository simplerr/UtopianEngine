#include "core/renderer/RendererUtility.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Pipeline.h"
#include "utopian/utility/Utility.h"
#include "utopian/core/Log.h"
#include <fstream>
#include <utility/Utility.h>
#include "ktx.h"
#include "ktxVulkan.h"
#include "core/renderer/Primitive.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace Utopian
{
   // Can be found in external\gli\gli\gl.hpp
   #define GL_RGBA32F 0x8814     // same as GL_RGBA32F_EXT and GL_RGBA32F_ARB
   #define GL_R32F    0x822E

   RendererUtility& gRendererUtility()
   {
      return RendererUtility::Instance();
   }

   void RendererUtility::DrawFullscreenQuad(Vk::CommandBuffer* commandBuffer)
   {
      commandBuffer->CmdDraw(3, 1, 0, 0);
   }

   void RendererUtility::DrawPrimitive(Vk::CommandBuffer* commandBuffer, Primitive* primitive)
   {
      commandBuffer->CmdBindVertexBuffer(0, 1, primitive->GetVertxBuffer());

      if (primitive->GetNumIndices() > 0)
      {
         commandBuffer->CmdBindIndexBuffer(primitive->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
         commandBuffer->CmdDrawIndexed(primitive->GetNumIndices(), 1, 0, 0, 0);
      }
      else
         commandBuffer->CmdDraw(primitive->GetNumVertices(), 1, 0, 0);
   }

   void RendererUtility::SetAdditiveBlending(VkPipelineColorBlendAttachmentState& blendAttachmentState)
   {
      // Enable additive blending
      blendAttachmentState.blendEnable = VK_TRUE;
      blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
      blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
      blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
      blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
      blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
      blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
   }

   void RendererUtility::SetAlphaBlending(VkPipelineColorBlendAttachmentState& blendAttachmentState)
   {
      blendAttachmentState.blendEnable = VK_TRUE;
      blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
      blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
      blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
      blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
   }

   void RendererUtility::SaveToFile(Vk::Device* device, const SharedPtr<Vk::Image>& image, std::string filename, uint32_t width, uint32_t height, VkFormat format)
   {
      SharedPtr<Vk::Image> hostVisibleImage = CreateHostVisibleImage(device, image, width, height, format);

      // Get layout of the image (including row pitch)
      VkImageSubresource subResource{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
      VkSubresourceLayout subResourceLayout;
      vkGetImageSubresourceLayout(device->GetVkDevice(), hostVisibleImage->GetVkHandle(), &subResource, &subResourceLayout);

      const char* data;
      hostVisibleImage->MapMemory((void**)&data);
      data += subResourceLayout.offset;

      if (GetFileExtension(filename) == ".ktx")
         SaveToFileKtx(filename, data, width, height, subResourceLayout, format);
      else if (GetFileExtension(filename) == ".ppm")
         SaveToFilePpm(filename, data, width, height, subResourceLayout);
      else if (GetFileExtension(filename) == ".png")
         stbi_write_png(filename.c_str(), width, height, 4, data, width * 4u);
      else
         UTO_LOG("Unsupported file extension: " + filename);

      hostVisibleImage->UnmapMemory();
   }

   void RendererUtility::SaveToFileKtx(std::string filename, const char* data, uint32_t width, uint32_t height, VkSubresourceLayout layout, VkFormat format)
   {
      ktxTexture* texture;
      ktxTextureCreateInfo createInfo;

      if (format == VK_FORMAT_R32_SFLOAT)
         createInfo.glInternalformat = GL_R32F;
      else if (format == VK_FORMAT_R32G32B32A32_SFLOAT)
         createInfo.glInternalformat = GL_RGBA32F;
      else
         assert(0);

      createInfo.baseWidth = 512;
      createInfo.baseHeight = 512;
      createInfo.baseDepth = 1;
      createInfo.numDimensions = 2;
      createInfo.numLevels = 1;
      createInfo.numLayers = 1;
      createInfo.numFaces = 1;
      createInfo.isArray = KTX_FALSE;
      createInfo.generateMipmaps = KTX_FALSE;
      KTX_error_code result = ktxTexture_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture);

      const ktx_uint8_t* src = (const ktx_uint8_t*)data;
      ktx_size_t srcSize = layout.size;
      ktx_uint32_t level = 0, layer = 0, faceSlice = 0;
      result = ktxTexture_SetImageFromMemory(texture, level, layer, faceSlice, src, srcSize);

      ktxTexture_WriteToNamedFile(texture, filename.c_str());
      ktxTexture_Destroy(texture);
   }

   void RendererUtility::SaveToFilePpm(std::string filename, const char* data, uint32_t width, uint32_t height, VkSubresourceLayout layout)
   {
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
         data += layout.rowPitch;
      }

      file.close();
   }

   SharedPtr<Vk::Image> RendererUtility::CreateHostVisibleImage(Vk::Device* device, const SharedPtr<Vk::Image>& srcImage, uint32_t width, uint32_t height, VkFormat format)
   {
      Vk::IMAGE_CREATE_INFO info;
      info.width = width;
      info.height = height;
      info.tiling = VK_IMAGE_TILING_LINEAR;
      info.format = format;
      info.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      info.finalImageLayout = VK_IMAGE_LAYOUT_GENERAL;
      info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
      info.name = "Host visible image";

      SharedPtr<Vk::Image> hostVisibleImage = std::make_shared<Vk::Image>(info, device);
      hostVisibleImage->SetFinalLayout(info.finalImageLayout);

      gRendererUtility().CopyImage(device, *hostVisibleImage, *srcImage);

      return hostVisibleImage;
   }

   void RendererUtility::CopyImage(Vk::Device* device, Vk::Image& dstImage, Vk::Image& srcImage)
   {
      bool supportsBlit = true;

      VkFormatProperties formatProps;

      // Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
      vkGetPhysicalDeviceFormatProperties(device->GetPhysicalDevice(), srcImage.GetFormat(), &formatProps);
      if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
         supportsBlit = false;
      }

      // Check if the device supports blitting to linear images
      vkGetPhysicalDeviceFormatProperties(device->GetPhysicalDevice(), dstImage.GetFormat(), &formatProps);
      if (!(formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
         supportsBlit = false;
      }

      // When using vkCmdCopyImage the images needs to have the same dimensions and format
      if (!supportsBlit && (dstImage.GetWidth() != srcImage.GetWidth() ||
         dstImage.GetHeight() != srcImage.GetHeight() ||
         srcImage.GetFormat() != dstImage.GetFormat())) {
         assert(0);
      }

      Vk::CommandBuffer commandBuffer = Vk::CommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

      dstImage.LayoutTransition(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
      srcImage.LayoutTransition(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

      if (supportsBlit)
         srcImage.Blit(commandBuffer, &dstImage);
      else
         srcImage.Copy(commandBuffer, &dstImage);

      dstImage.LayoutTransition(commandBuffer, dstImage.GetFinalLayout());
      srcImage.LayoutTransition(commandBuffer, srcImage.GetFinalLayout());

      commandBuffer.Flush();
   }
}