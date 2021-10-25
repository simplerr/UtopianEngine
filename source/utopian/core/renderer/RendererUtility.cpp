#include "core/renderer/RendererUtility.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Pipeline.h"
#include "vulkan/EffectManager.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/Texture.h"
#include "vulkan/RenderTarget.h"
#include "utopian/utility/Utility.h"
#include "utopian/core/Log.h"
#include "utopian/core/ModelLoader.h"
#include <core/Engine.h>
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

   RendererUtility::RendererUtility()
   {
      mCubemapModel = gModelLoader().LoadBox();
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

   void RendererUtility::FilterCubemap(Vk::Texture* inputCubemap, Vk::Texture* outputCubemap, std::string filterShader)
   {
      Vk::Device* device = gEngine().GetVulkanApp()->GetDevice();

      const VkFormat format = outputCubemap->GetImage().GetFormat();
      const uint32_t dimension = outputCubemap->GetWidth();
      const uint32_t numMipLevels = static_cast<uint32_t>(floor(log2(dimension))) + 1;

      // Offscreen framebuffer
      SharedPtr<Vk::Image> offscreen = std::make_shared<Vk::ImageColor>(device, dimension,
         dimension, format, "Offscreen irradiance image");

      SharedPtr<Vk::RenderTarget> renderTarget = std::make_shared<Vk::RenderTarget>(device, dimension, dimension);
      renderTarget->AddWriteOnlyColorAttachment(offscreen);
      renderTarget->SetClearColor(1, 1, 1, 1);
      renderTarget->Create();

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/ibl_filtering/cubemap_filter.vert";
      effectDesc.shaderDesc.fragmentShaderPath = filterShader;
      SharedPtr<Vk::Effect> effect = Vk::gEffectManager().AddEffect<Vk::Effect>(device, renderTarget->GetRenderPass(), effectDesc);

      effect->BindCombinedImage("samplerEnv", *inputCubemap);

      //SharedPtr<Vk::Texture> outputCubemap = Vk::gTextureLoader().CreateCubemapTexture(format, dimension, dimension, numMipLevels);

      glm::mat4 matrices[] =
      {
         glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
         glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
         glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
         glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
         glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
         glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
      };

      Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();
      commandBuffer->Begin();
      renderTarget->BeginDebugLabelAndQueries("Irradiance cubemap generation", glm::vec4(1.0f));

      outputCubemap->GetImage().LayoutTransition(*commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

      for (uint32_t mipLevel = 0; mipLevel < numMipLevels; mipLevel++)
      {
         for (uint32_t face = 0; face < 6; face++)
         {
            renderTarget->BeginRenderPass();

            float viewportSize = dimension * std::pow(0.5f, mipLevel);
            commandBuffer->CmdSetViewPort(viewportSize, viewportSize);

            struct PushConsts {
               glm::mat4 mvp;
               float roughness;
            } pushConsts;

            glm::mat4 world = glm::scale(glm::mat4(), glm::vec3(10000.0f));
            pushConsts.mvp = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, 20000.0f) * matrices[face] * world;
            pushConsts.roughness = (float)mipLevel / (float)(numMipLevels - 1);
            commandBuffer->CmdPushConstants(effect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(PushConsts), &pushConsts.mvp);

            commandBuffer->CmdBindPipeline(effect->GetPipeline());
            commandBuffer->CmdBindDescriptorSets(effect);

            Primitive* primitive = mCubemapModel->GetPrimitive(0);
            commandBuffer->CmdBindVertexBuffer(0, 1, primitive->GetVertxBuffer());
            commandBuffer->CmdBindIndexBuffer(primitive->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
            commandBuffer->CmdDrawIndexed(primitive->GetNumIndices(), 1, 0, 0, 0);

            commandBuffer->CmdEndRenderPass();

            // Copy region for transfer from framebuffer to cube face
            VkImageCopy copyRegion = {};

            copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.srcSubresource.baseArrayLayer = 0;
            copyRegion.srcSubresource.mipLevel = 0;
            copyRegion.srcSubresource.layerCount = 1;
            copyRegion.srcOffset = { 0, 0, 0 };

            copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.dstSubresource.baseArrayLayer = face;
            copyRegion.dstSubresource.mipLevel = mipLevel;
            copyRegion.dstSubresource.layerCount = 1;
            copyRegion.dstOffset = { 0, 0, 0 };

            copyRegion.extent.width = static_cast<uint32_t>(viewportSize);
            copyRegion.extent.height = static_cast<uint32_t>(viewportSize);
            copyRegion.extent.depth = 1;

            offscreen->LayoutTransition(*commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

            vkCmdCopyImage(commandBuffer->GetVkHandle(), offscreen->GetVkHandle(),
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, outputCubemap->GetImage().GetVkHandle(),
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

            offscreen->LayoutTransition(*commandBuffer, offscreen->GetFinalLayout());
         }
      }

      outputCubemap->GetImage().LayoutTransition(*commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      renderTarget->EndDebugLabelAndQueries();
      commandBuffer->Flush();
   }
}