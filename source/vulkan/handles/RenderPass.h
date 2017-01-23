#pragma once

#include <vulkan/vulkan.h>
#include "Handle.h"

namespace VulkanLib
{
	class Device;

	class RenderPass : public Handle<VkRenderPass>
	{
	public:
		RenderPass(Device* device, VkFormat colorFormat, VkFormat depthFormat);
	private:
	};
}
