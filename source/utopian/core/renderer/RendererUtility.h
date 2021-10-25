#pragma once

#include "utility\Module.h"
#include "utility\Common.h"
#include "vulkan\VulkanPrerequisites.h"
#include <string>

namespace Utopian
{
   class Model;

   class RendererUtility : public Module<RendererUtility>
   {
   public:
      RendererUtility();

      void DrawFullscreenQuad(Vk::CommandBuffer* commandBuffer);
      void DrawPrimitive(Vk::CommandBuffer* commandBuffer, Primitive* primitive);
      //void DrawMesh(...);

      /** Blend state helpers. */
      void SetAdditiveBlending(VkPipelineColorBlendAttachmentState& blendAttachmentState);
      void SetAlphaBlending(VkPipelineColorBlendAttachmentState& blendAttachmentState);

      /** Functions for copying images. */
      void CopyImage(Vk::Device* device, Vk::Image& dstImage, Vk::Image& srcImage);

      void SaveToFile(Vk::Device* device, const SharedPtr<Vk::Image>& image, std::string filename,
                      uint32_t width, uint32_t height, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);

      SharedPtr<Vk::Image> CreateHostVisibleImage(Vk::Device* device, const SharedPtr<Vk::Image>& srcImage,
                                                  uint32_t width, uint32_t height, VkFormat format);

      void FilterCubemap(Vk::Texture* inputCubemap, Vk::Texture* outputCubemap, std::string filterShader);

   private:
      void SaveToFileKtx(std::string filename, const char* data, uint32_t width, uint32_t height, VkSubresourceLayout layout, VkFormat format);
      void SaveToFilePpm(std::string filename, const char* data, uint32_t width, uint32_t height, VkSubresourceLayout layout);

   private:
      SharedPtr<Model> mCubemapModel;
   };

   RendererUtility& gRendererUtility();
}
