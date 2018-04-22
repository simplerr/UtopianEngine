#pragma once

#include <vector>
#include "Handle.h"
#include "vulkan/VulkanInclude.h"

namespace Utopian::Vk
{
	enum RenderPassAttachment
	{
		COLOR_ATTACHMENT = 0,
		DEPTH_ATTACHMENT
	};

	class RenderPass : public Handle<VkRenderPass>
	{
	public:
		/**
		@param create If false then the Create() function has to to be called manually after the constructor,
			          this allows for modifying the renderpass configuration.
		*/
		RenderPass(Device* device, VkFormat colorFormat, VkFormat depthFormat, VkImageLayout colorImageLayout, bool create = true);
		RenderPass(Device* device);
		void Create();

		// Note: The order of which these are called is important!
		void AddColorAttachment(VkFormat format, VkImageLayout imageLayout);
		void AddDepthAttachment(VkFormat format, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		uint32_t GetNumColorAttachments() const;

		// Descriptors for the attachments used by this renderpass
		// This is public so that it can be modified before creating the render pass
		std::vector<VkAttachmentDescription> attachments;
		std::vector<VkAttachmentReference> colorReferences;
		std::vector<VkAttachmentReference> depthReferences;
	};
}
