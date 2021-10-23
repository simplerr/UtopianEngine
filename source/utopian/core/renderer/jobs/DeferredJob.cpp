#include "core/renderer/jobs/DeferredJob.h"
#include "core/renderer/jobs/BlurJob.h"
#include "core/renderer/jobs/ShadowJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "vulkan/Effect.h"
#include <core/renderer/RenderSettings.h>

namespace Utopian
{
   DeferredJob::DeferredJob(Vk::Device* device, uint32_t width, uint32_t height)
      : BaseJob(device, width, height)
   {
      renderTarget = std::make_shared<Vk::BasicRenderTarget>(device, width, height, VK_FORMAT_R32G32B32A32_SFLOAT);

      Vk::EffectCreateInfo effectDescPhong;
      effectDescPhong.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
      effectDescPhong.shaderDesc.fragmentShaderPath = "data/shaders/deferred/deferred_phong.frag";

      Vk::EffectCreateInfo effectDescPbr;
      effectDescPbr.shaderDesc.vertexShaderPath = "data/shaders/common/fullscreen.vert";
      effectDescPbr.shaderDesc.fragmentShaderPath = "data/shaders/deferred/deferred_pbr.frag";

      mPhongEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(device, renderTarget->GetRenderPass(), effectDescPhong);
      mPbrEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(device, renderTarget->GetRenderPass(), effectDescPbr);

      // Create sampler that returns 1.0 when sampling outside the depth image
      mDepthSampler = std::make_shared<Vk::Sampler>(device, false);
      mDepthSampler->createInfo.anisotropyEnable = VK_FALSE; // Anistropic filter causes artifacts at the edge between cascades
      mDepthSampler->createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
      mDepthSampler->createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
      mDepthSampler->createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
      mDepthSampler->createInfo.borderColor = VkBorderColor::VK_BORDER_COLOR_INT_OPAQUE_WHITE;
      mDepthSampler->Create();

      mSampler = std::make_shared<Vk::Sampler>(mDevice);

      //mScreenQuad = gScreenQuadUi().AddQuad(0u, 0u, width, height, renderTarget->GetColorImage().get(), renderTarget->GetSampler(), 1u);
   }

   DeferredJob::~DeferredJob()
   {
   }

   void DeferredJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
   {
      BlurJob* blurJob = static_cast<BlurJob*>(jobs[JobGraph::BLUR_INDEX]);
      ShadowJob* shadowJob = static_cast<ShadowJob*>(jobs[JobGraph::SHADOW_INDEX]);

      light_ubo.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      settings_ubo.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      cascade_ubo.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      atmosphere_ubo.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

      mPhongEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
      mPhongEffect->BindUniformBuffer("UBO_lights", light_ubo);
      mPhongEffect->BindUniformBuffer("UBO_settings", settings_ubo);
      mPhongEffect->BindUniformBuffer("UBO_cascades", cascade_ubo);
      mPhongEffect->BindUniformBuffer("UBO_atmosphere", atmosphere_ubo);

      mPhongEffect->BindCombinedImage("positionSampler", *gbuffer.positionImage, *mSampler);
      mPhongEffect->BindCombinedImage("normalSampler", *gbuffer.normalImage, *mSampler);
      mPhongEffect->BindCombinedImage("albedoSampler", *gbuffer.albedoImage, *mSampler);
      mPhongEffect->BindCombinedImage("ssaoSampler", *blurJob->blurImage, *mSampler);
      mPhongEffect->BindCombinedImage("pbrSampler", *gbuffer.pbrImage, *mSampler);
      mPhongEffect->BindCombinedImage("shadowSampler", *shadowJob->depthColorImage, *mDepthSampler);

      mPbrEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
      mPbrEffect->BindUniformBuffer("UBO_lights", light_ubo);
      mPbrEffect->BindUniformBuffer("UBO_settings", settings_ubo);
      mPbrEffect->BindUniformBuffer("UBO_cascades", cascade_ubo);
      mPbrEffect->BindUniformBuffer("UBO_atmosphere", atmosphere_ubo);

      mPbrEffect->BindCombinedImage("positionSampler", *gbuffer.positionImage, *mSampler);
      mPbrEffect->BindCombinedImage("normalSampler", *gbuffer.normalImage, *mSampler);
      mPbrEffect->BindCombinedImage("albedoSampler", *gbuffer.albedoImage, *mSampler);
      mPbrEffect->BindCombinedImage("ssaoSampler", *blurJob->blurImage, *mSampler);
      mPbrEffect->BindCombinedImage("pbrSampler", *gbuffer.pbrImage, *mSampler);
      mPbrEffect->BindCombinedImage("shadowSampler", *shadowJob->depthColorImage, *mDepthSampler);

      mIbl.environmentMap = Vk::gTextureLoader().LoadCubemapTexture("data/textures/environments/papermill.ktx", VK_FORMAT_R16G16B16A16_SFLOAT);

      mIbl.irradianceMap = gRendererUtility().FilterCubemap(mIbl.environmentMap.get(), 64, VK_FORMAT_R32G32B32A32_SFLOAT,
                           "data/shaders/ibl_filtering/irradiance_filter.frag");

      mIbl.specularMap = gRendererUtility().FilterCubemap(mIbl.environmentMap.get(), 512, VK_FORMAT_R16G16B16A16_SFLOAT,
                         "data/shaders/ibl_filtering/specular_filter.frag");

      mIbl.brdfLut = Vk::gTextureLoader().LoadTexture("data/textures/brdf_lut.ktx", VK_FORMAT_R16G16_SFLOAT);
      mIbl.brdfLut->GetSampler().createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      mIbl.brdfLut->GetSampler().createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      mIbl.brdfLut->GetSampler().createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      mIbl.brdfLut->GetSampler().Create();
      mIbl.brdfLut->UpdateDescriptor();

      mPbrEffect->BindCombinedImage("irradianceMap", *mIbl.irradianceMap);
      mPbrEffect->BindCombinedImage("specularMap", *mIbl.specularMap);
      mPbrEffect->BindCombinedImage("brdfLut", *mIbl.brdfLut);
   }

   void DeferredJob::Render(const JobInput& jobInput)
   {
      // Settings data
      settings_ubo.data.fogColor = jobInput.renderingSettings.fogColor;
      settings_ubo.data.fogStart = jobInput.renderingSettings.fogStart;
      settings_ubo.data.fogDistance = jobInput.renderingSettings.fogDistance;
      settings_ubo.data.cascadeColorDebug = jobInput.renderingSettings.cascadeColorDebug;
      settings_ubo.data.useIBL = jobInput.renderingSettings.iblEnabled;

      settings_ubo.UpdateMemory();

      // Light array
      light_ubo.lights.clear();
      for (auto& light : jobInput.sceneInfo.lights)
      {
         light_ubo.lights.push_back(light->GetLightData());
      }

      light_ubo.constants.numLights = (float)light_ubo.lights.size();
      light_ubo.UpdateMemory();

      // Note: Todo: Temporary
      for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
      {
         cascade_ubo.data.cascadeSplits[i] = jobInput.sceneInfo.cascades[i].splitDepth;
         cascade_ubo.data.cascadeViewProjMat[i] = jobInput.sceneInfo.cascades[i].viewProjMatrix;
      }

      // Note: This should probably be moved. We need the fragment position in view space
      // when comparing it's Z value to find out which shadow map cascade it should sample from.
      cascade_ubo.data.cameraViewMat = jobInput.sceneInfo.viewMatrix;
      cascade_ubo.data.shadowSampleSize = jobInput.renderingSettings.shadowSampleSize;
      cascade_ubo.data.shadowsEnabled = jobInput.renderingSettings.shadowsEnabled;
      cascade_ubo.UpdateMemory();

      // Todo: Reuse from AtmosphereJob, see Issue #127
      if (jobInput.renderingSettings.sky == SKY_ATMOSPHERE)
         atmosphere_ubo.data.atmosphericScattering = true;
      else
         atmosphere_ubo.data.atmosphericScattering = false;

      atmosphere_ubo.data.sunDir = jobInput.sceneInfo.directionalLight->GetDirection();
      atmosphere_ubo.UpdateMemory();

      // End of temporary

      renderTarget->Begin("Deferred pass", glm::vec4(0.0, 1.0, 1.0, 1.0));
      Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

      SharedPtr<Vk::Effect> effect = nullptr;
      if (jobInput.renderingSettings.shadingMethod == ShadingMethod::PHONG)
         effect = mPhongEffect;
      else
         effect = mPbrEffect;

      // Todo: Should this be moved to the effect instead?
      commandBuffer->CmdBindPipeline(effect->GetPipeline());
      commandBuffer->CmdBindDescriptorSets(effect);

      gRendererUtility().DrawFullscreenQuad(commandBuffer);

      renderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
   }
}
