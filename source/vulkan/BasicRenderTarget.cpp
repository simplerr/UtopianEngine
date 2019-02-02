#include "vulkan/BasicRenderTarget.h"
#include "vulkan/handles/Image.h"

namespace Utopian::Vk
{
	BasicRenderTarget::BasicRenderTarget(Device* device, uint32_t width, uint32_t height, VkFormat colorFormat, VkFormat depthFormat)
		: RenderTarget(device,  width, height)
	{
		mColorImage = new ImageColor(device, width, height, colorFormat);
		mDepthImage = new ImageDepth(device, width, height, depthFormat);

		AddColorAttachment(mColorImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		AddDepthAttachment(mDepthImage);
		Create();
	}

	BasicRenderTarget::~BasicRenderTarget()
	{
		delete mColorImage;
		delete mDepthImage;
	}

	Image* BasicRenderTarget::GetColorImage()
	{
		return mColorImage;
	}

	Image* BasicRenderTarget::GetDepthImage()
	{
		return mDepthImage;
	}
}
