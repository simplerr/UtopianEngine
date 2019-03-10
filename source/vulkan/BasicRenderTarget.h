#pragma once

#include <stdint.h>
#include <glm/glm.hpp>
#include <vector>
#include "vulkan/VulkanInclude.h"
#include "vulkan/RenderTarget.h"
#include "utility/Common.h"

namespace Utopian::Vk
{
	/** 
	 * A render target that has one built in color and depth attachment.
	 */
	class BasicRenderTarget : public RenderTarget
	{
	public:
		BasicRenderTarget(Device* device, uint32_t width, uint32_t height, VkFormat colorFormat, VkFormat depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT);
		~BasicRenderTarget();

		SharedPtr<Image>& GetColorImage();
		SharedPtr<Image>& GetDepthImage();
	private:
		SharedPtr<Image> mColorImage;
		SharedPtr<Image> mDepthImage;
	};
}
