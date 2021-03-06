#include "vulkan/BasicRenderTarget.h"
#include "vulkan/handles/Image.h"

namespace Utopian::Vk
{
   BasicRenderTarget::BasicRenderTarget(Device* device, uint32_t width, uint32_t height, VkFormat colorFormat, VkFormat depthFormat)
      : RenderTarget(device,  width, height)
   {
      mColorImage = std::make_shared<ImageColor>(device, width, height, colorFormat, "Basic render target color image");
      mDepthImage = std::make_shared<ImageDepth>(device, width, height, depthFormat, "Basic render target depth image");

      AddWriteOnlyColorAttachment(mColorImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      AddWriteOnlyDepthAttachment(mDepthImage);
      Create();
   }

   BasicRenderTarget::~BasicRenderTarget()
   {

   }

   SharedPtr<Image>& BasicRenderTarget::GetColorImage()
   {
      return mColorImage;
   }

   SharedPtr<Image>& BasicRenderTarget::GetDepthImage()
   {
      return mDepthImage;
   }
}
