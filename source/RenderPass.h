#pragma once
#include "Handle.h"
#include <vulkan/vulkan.h>

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
