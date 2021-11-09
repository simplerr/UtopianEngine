#pragma once
#include "core/renderer/jobs/BaseJob.h"
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/Texture.h"
#include "core/renderer/jobs/JobGraph.h"
#include <glm/glm.hpp>
#include <string>

namespace Utopian
{
   class Terrain;

   class GBufferTerrainJob : public BaseJob
   {
   public:
      UNIFORM_BLOCK_BEGIN(FrustumPlanes)
         UNIFORM_PARAM(glm::vec4, frustumPlanes[6])
      UNIFORM_BLOCK_END()

      UNIFORM_BLOCK_BEGIN(SettingsBlock)
         UNIFORM_PARAM(glm::vec2, viewportSize)
         UNIFORM_PARAM(float, edgeSize)
         UNIFORM_PARAM(float, tessellationFactor)
         UNIFORM_PARAM(float, amplitude)
         UNIFORM_PARAM(float, textureScaling)
         UNIFORM_PARAM(float, bumpmapAmplitude)
         UNIFORM_PARAM(int, wireframe)
      UNIFORM_BLOCK_END()

      GBufferTerrainJob(Vk::Device* device, Terrain* terrain, uint32_t width, uint32_t height);
      ~GBufferTerrainJob();

      void LoadResources() override;

      void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
      void PostInit(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
      void Render(const JobInput& jobInput) override;
      void Update() override;

      SharedPtr<Vk::RenderTarget> renderTarget;
   private:
      SharedPtr<Vk::Effect> mEffect;
      SharedPtr<Vk::QueryPoolStatistics> mQueryPool;
      SharedPtr<Vk::Sampler> mSampler;
      FrustumPlanes mFrustumPlanesBlock;
      SettingsBlock mSettingsBlock;
      Terrain::BrushBlock mBrushBlock;
      Terrain* mTerrain;

      Vk::TextureArray mDiffuseTextureArray;
      Vk::TextureArray mNormalTextureArray;
      Vk::TextureArray mDisplacementTextureArray;
   };
}
