#include "core/renderer/jobs/PixelDebugJob.h"
#include "core/renderer/jobs/GeometryThicknessJob.h"
#include "core/renderer/jobs/SSRJob.h"
#include "core/renderer/jobs/SSAOJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "core/renderer/ImGuiRenderer.h"
#include "core/Input.h"
#include "core/Camera.h"
#include "core/renderer/Renderer.h"
#include "im3d/im3d.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Utopian
{
   PixelDebugJob::PixelDebugJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {

   }

   PixelDebugJob::~PixelDebugJob()
   {
   }

   void PixelDebugJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      mRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
      mRenderTarget->SetClearColor(0, 1, 0, 1);
      mRenderTarget->Create();

      Vk::EffectCreateInfo effectDesc;
      effectDesc.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
      effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/pixel_debug/pixel_debug.frag";

      mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), effectDesc);

      mMouseInputBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      mOutputBuffer.Create(mDevice, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

      mEffect->BindUniformBuffer("UBO_input", mMouseInputBlock);
      mEffect->BindStorageBuffer("UBO_output", mOutputBuffer.GetDescriptor());

      // Update this to the image to read pixel values from
      SSRJob* ssrJob = static_cast<SSRJob*>(jobs[JobGraph::SSR_INDEX]);
      //mEffect->BindCombinedImage("debugSampler", ssrJob->rayOriginImage, mRenderTarget->GetSampler());
      //mEffect->BindCombinedImage("debugSampler2", ssrJob->rayEndImage, mRenderTarget->GetSampler());
      //mEffect->BindCombinedImage("debugSampler3", ssrJob->miscDebugImage, mRenderTarget->GetSampler());

      glm::vec2 mousePos = gInput().GetMousePosition();
      mMouseInputBlock.data.mousePosUV = glm::vec2(mousePos.x / mWidth, mousePos.y / mHeight);
      mMouseInputBlock.UpdateMemory();
   }

   void PixelDebugJob::Render(const JobInput& jobInput)
   {
      if (gInput().KeyDown(VK_LBUTTON))
      {
         glm::vec2 mousePos = gInput().GetMousePosition();
         mMouseInputBlock.data.mousePosUV = glm::vec2(mousePos.x / mWidth, mousePos.y / mHeight);
         mMouseInputBlock.UpdateMemory();
      }

      mRenderTarget->Begin("Pixel debug pass", glm::vec4(0.5, 1.0, 0.3, 1.0));
      Vk::CommandBuffer* commandBuffer = mRenderTarget->GetCommandBuffer();

      if (IsEnabled())
      {
         commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
         commandBuffer->CmdBindDescriptorSets(mEffect);
         gRendererUtility().DrawFullscreenQuad(commandBuffer);

         // Retrieve pixel value
         glm::vec4* mapped;
         mOutputBuffer.MapMemory(0, 3 * sizeof(glm::vec4), 0, (void**)& mapped);
         mOutputBuffer.data.pixelValue = mapped[0];
         mOutputBuffer.data.pixelValue2 = mapped[1];
         mOutputBuffer.data.pixelValue3 = mapped[2];
         mOutputBuffer.UnmapMemory();
      }

      mRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
   }

   void PixelDebugJob::Update()
   {
      return;

      glm::vec4 pixelValue = mOutputBuffer.data.pixelValue;
      glm::vec4 pixelValue2 = mOutputBuffer.data.pixelValue2;
      glm::vec4 pixelValue3 = mOutputBuffer.data.pixelValue3;
      ImGuiRenderer::BeginWindow("Pixel debug", glm::vec2(500.0f, 10.0f), 400.0f);
      ImGuiRenderer::TextV("R: %f, G: %f, B: %f, A: %f", pixelValue.x, pixelValue.y, pixelValue.z, pixelValue.w);
      ImGuiRenderer::TextV("R: %f, G: %f, B: %f, A: %f", pixelValue2.x, pixelValue2.y, pixelValue2.z, pixelValue2.w);
      ImGuiRenderer::TextV("R: %f, G: %f, B: %f, A: %f", pixelValue3.x, pixelValue3.y, pixelValue3.z, pixelValue3.w);
      ImGuiRenderer::EndWindow();

      static glm::vec3 cameraPos, rayOrigin, rayEnd;

      if (gInput().KeyDown(VK_LBUTTON))
      {
         cameraPos = gRenderer().GetMainCamera()->GetPosition();
         glm::mat4 inverseView = glm::inverse(gRenderer().GetMainCamera()->GetView());
         rayOrigin = inverseView * glm::vec4(glm::vec3(pixelValue), 1.0f);
         rayEnd = inverseView * glm::vec4(glm::vec3(pixelValue2), 1.0f);

         rayOrigin *= -1;
         rayEnd *= -1;
      }

      Im3d::DrawLine(cameraPos, rayOrigin, 3.0f, Im3d::Color(0, 1, 0));
      Im3d::DrawLine(glm::vec3(rayOrigin), glm::vec3(rayEnd), 3.0f, Im3d::Color(1, 0, 0));
      Im3d::DrawLine(glm::vec3(0), glm::vec3(1000), 3.0f, Im3d::Color(1, 0, 0));
   }
}
