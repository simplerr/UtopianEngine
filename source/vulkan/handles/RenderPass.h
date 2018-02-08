#pragma once

#include <array>
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
		void Create();
		
		// Descriptors for the attachments used by this renderpass
		// This is public so that it can be modified before creating the render pass
		std::array<VkAttachmentDescription, 2> attachments = {};
	};
}
