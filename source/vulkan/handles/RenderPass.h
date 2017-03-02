#pragma once

#include <array>
#include <vulkan/vulkan.h>
#include "Handle.h"

namespace VulkanLib
{
	class Device;

	enum RenderPassAttachment
	{
		COLOR_ATTACHMENT = 0,
		DEPTH_ATTACHMENT
	};

	class RenderPass : public Handle<VkRenderPass>
	{
	public:
		RenderPass(Device* device, VkFormat colorFormat, VkFormat depthFormat);
		void Create();
		
		// Descriptors for the attachments used by this renderpass
		// This is public so that it can be modified before creating the render pass
		std::array<VkAttachmentDescription, 2> attachments = {};
	};
}
