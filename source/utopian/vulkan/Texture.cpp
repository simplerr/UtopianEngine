#include "vulkan/Texture.h"
#include "vulkan/handles/Sampler.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/Image.h"

namespace Utopian::Vk
{
   Texture::Texture(Device* device)
   {
      mDevice = device;
   }

   Texture::~Texture()
   {
   }

   const VkDescriptorImageInfo* Texture::GetDescriptor() const
   {
      return &mDescriptor;
   }

   Image& Texture::GetImage()
   {
      return *mImage;
   }
   
   Sampler& Texture::GetSampler()
   {
      return *mSampler;
   }

   void Texture::SetPath(std::string path)
   {
      mPath = path;
   }

   std::string Texture::GetPath() const
   {
      return mPath;
   }

   uint32_t Texture::GetWidth() const
   {
      return mWidth;
   }

   uint32_t Texture::GetHeight() const
   {
      return mHeight;
   }

   uint32_t Texture::GetNumMipLevels() const
   {
      return mNumMipLevels;
   }

   void Texture::UpdateDescriptor()
   {
      mDescriptor.sampler = mSampler->GetVkHandle();
      mDescriptor.imageView = mImage->GetView();
      mDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   }

   TextureArray::TextureArray()
   {

   }

   void TextureArray::AddTexture(const SharedPtr<Vk::Texture>& texture)
   {
      VkDescriptorImageInfo imageInfo;
      imageInfo.sampler = texture->GetSampler().GetVkHandle();
      imageInfo.imageView = texture->GetImage().GetView();
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

      mImageInfos.push_back(imageInfo);
   }

   const VkDescriptorImageInfo* TextureArray::GetDescriptor() const
   {
      return mImageInfos.data();
   }
   
   uint32_t TextureArray::GetNumImages() const
   {
      return (uint32_t)mImageInfos.size();
   }
}
