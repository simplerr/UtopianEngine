#pragma once
#include "vulkan/VulkanPrerequisites.h"
#include "im3d/im3d.h"
#include "vulkan/ShaderBuffer.h"
#include "utility/Common.h"
#include "glm/glm.hpp"

namespace Utopian
{
   class Im3dRenderer
   {
   public:
      Im3dRenderer(Vk::VulkanApp* vulkanApp, glm::vec2 viewportSize);
      ~Im3dRenderer();

      void NewFrame();
      void EndFrame();

      /** Uploads the Im3d generated vertex buffer to the GPU. Actual rendering is done in Im3dJob. */
      void UploadVertexData();

      SharedPtr<Vk::Buffer> GetVertexBuffer();
   private:
      uint32_t GetTotalNumVertices();
   private:
      Vk::VulkanApp* mVulkanApp;
      Im3d::VertexData* mMappedVertices;
      glm::vec2 mViewportSize;
      SharedPtr<Vk::Buffer> mVertexBuffer; // Contains all vertices created by Im3d, when rendering offsets are used in this buffer.
      uint32_t mVertexCount;
   };
}
