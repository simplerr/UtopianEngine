#pragma once

#include <vector>
#include "Handle.h"
#include "vulkan/VulkanPrerequisites.h"

namespace Utopian::Vk
{
   /**
    * Todo: Note: These indexes are not correct if a render pass have more
    * than one color attachment.
    */
   enum RenderPassAttachment
   {
      COLOR_ATTACHMENT = 0,
      DEPTH_ATTACHMENT
   };

   /** Wrapper for VkRenderPass. */
   class RenderPass : public Handle<VkRenderPass>
   {
   public:
      RenderPass(Device* device);

      /**
       * Constructor that adds one color and one depth attachment.
       * @param create If false then the Create() function has to to be called manually after the constructor,
       * this allows for modifying the renderpass configuration.
       */
      RenderPass(Device* device, VkFormat colorFormat, VkFormat depthFormat, VkImageLayout colorImageLayout, bool create = true);

      /** Creates the render pass.
       * @note All needed attachments must be added before calling this.
       */
      void Create();

      /**
       * Adds attachments to the render pass.
       * @note The order of which these are called is important.
       */
      void AddColorAttachment(VkFormat format,
                              VkImageLayout finalImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                              VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                              VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                              VkImageLayout initialImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

      void AddDepthAttachment(VkFormat format,
                              VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                              VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                              VkImageLayout finalImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                              VkImageLayout initialImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

      uint32_t GetNumColorAttachments() const;

      /**
       * Descriptors for the attachments used by this renderpass
       * This is public so that it can be modified before creating the render pass
       */
      std::vector<VkAttachmentDescription> attachments;
      std::vector<VkAttachmentReference> colorReferences;
      std::vector<VkAttachmentReference> depthReferences;
   };
}
