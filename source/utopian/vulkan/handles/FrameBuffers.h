#pragma once

#include <vector>
#include "vulkan/VulkanPrerequisites.h"

class VulkanSwapChain;

namespace Utopian::Vk
{
   /**
    * Wrapper for handling framebuffers.
    * Supports both creating multiple framebuffer when used in the swapchain
    * and also manually adding multiple attachments to the same framebuffer.
    *
    * Todo: Note: It's a bit weird that this has special implementation when used
    * in the swapchain. Ugly that the number of framebuffers are hardcoded to 1
    * in the other case. For example the ShadowJob uses multiple framebuffers as well.
    *
    * Should perhaps just contain 1 VkFrameBuffer and then on a higher level have the vector.
    */
   class FrameBuffers
   {
   public:
      FrameBuffers(Device* device);

      /**
       * Special use case when used in the swapchain.
       * @note No need to call Create() after this.
       */
      FrameBuffers(Device* device, RenderPass* renderPass, Image* depthStencilImage, VulkanSwapChain* swapChain, uint32_t width, uint32_t height);
      ~FrameBuffers();

      /** Adds attachments to the framebuffer. */
      void AddAttachmentImage(Image* image);
      void AddAttachmentImage(VkImageView imageView);

      /** Creates the framebuffer. */
      void Create(RenderPass* renderPass, uint32_t width, uint32_t height);

      VkFramebuffer GetFrameBuffer(uint32_t index) const;
      VkFramebuffer GetCurrent() const;

      uint32_t currentFrameBuffer = 0;
   private:
      std::vector<VkImageView> mAttachments;
      std::vector<VkFramebuffer> mFrameBuffers;
      Device* mDevice;
   };
}
