#include "vulkan/BasicRenderTarget.h"
#include "vulkan/handles/Image.h"

namespace Utopian::Vk
{
	BasicRenderTarget::BasicRenderTarget(Device* device, CommandPool* commandPool, uint32_t width, uint32_t height, VkFormat colorFormat, VkFormat depthFormat)
		: RenderTarget(device, commandPool, width, height)
	{
		mColorImage = new ImageColor(device, width, height, VK_FORMAT_R8G8B8A8_UNORM);
		mDepthImage = new ImageDepth(device, width, height, VK_FORMAT_D32_SFLOAT_S8_UINT);

		AddColorAttachment(mColorImage);
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
