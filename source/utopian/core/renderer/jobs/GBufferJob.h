#pragma once

#include "core/renderer/jobs/BaseJob.h"
#include "vulkan/VulkanPrerequisites.h"

namespace Utopian
{
   #define NUM_MAX_SPHERES 64
   struct SphereInfo
   {
      glm::vec3 position;
      float radius;
   };

   class SphereUniformBuffer : public Utopian::Vk::ShaderBuffer
   {
   public:
      virtual void UpdateMemory()
      {
         uint8_t* mapped;
         uint32_t dataOffset = 0;
         uint32_t dataSize = sizeof(constants);
         mBuffer->MapMemory((void**)&mapped);
         memcpy(mapped, &constants.numSpheres, dataSize);
         mBuffer->UnmapMemory();

         dataOffset += dataSize;
         dataSize = (uint32_t)spheres.size() * sizeof(SphereInfo);
         mBuffer->MapMemory((void**)&mapped);
         memcpy(&mapped[dataOffset], spheres.data(), dataSize);
         mBuffer->UnmapMemory();
      }

      virtual int GetSize()
      {
         return (NUM_MAX_SPHERES) * sizeof(Utopian::SphereInfo) + sizeof(constants);
      }

      virtual std::string GetDebugName()
      {
         return "SphereUniformBuffer";
      }

      struct {
         float numSpheres;
         glm::vec3 padding;
      } constants;

      // Note: Todo:
      std::array<Utopian::SphereInfo, NUM_MAX_SPHERES> spheres;
   };

   class GBufferJob : public BaseJob
   {
   public:
      UNIFORM_BLOCK_BEGIN(SettingsBlock)
         UNIFORM_PARAM(int, normalMapping)
      UNIFORM_BLOCK_END()

      UNIFORM_BLOCK_BEGIN(AnimationParametersBlock)
         UNIFORM_PARAM(float, terrainSize)
         UNIFORM_PARAM(float, strength)
         UNIFORM_PARAM(float, frequency)
         UNIFORM_PARAM(int, enabled)
      UNIFORM_BLOCK_END()

      UNIFORM_BLOCK_BEGIN(FoliageSpheresBlock)
         UNIFORM_PARAM(float, numSpheres)
         UNIFORM_PARAM(glm::vec3, padding)
         UNIFORM_PARAM(SphereInfo, spheres[NUM_MAX_SPHERES])
      UNIFORM_BLOCK_END()

      struct InstancePushConstantBlock
      {
         InstancePushConstantBlock(float _modelHeight) {
            modelHeight = _modelHeight;
         }

         float modelHeight;
      };

      GBufferJob(Vk::Device* device, uint32_t width, uint32_t height);
      ~GBufferJob();

      void LoadResources() override;

      void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
      void PostInit(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
      void Render(const JobInput& jobInput) override;

   private:
      SharedPtr<Vk::RenderTarget> mRenderTarget;
      SharedPtr<Vk::Effect> mGBufferEffect;
      SharedPtr<Vk::Effect> mGBufferEffectWireframe;
      SharedPtr<Vk::Effect> mInstancedAnimationEffect;
      SharedPtr<Vk::Effect> mGBufferEffectInstanced;
      SharedPtr<Vk::Effect> mGBufferEffectSkinning;

      SettingsBlock mSettingsBlock;
      SphereUniformBuffer mFoliageSpheresBlock;

      // Animated instancing
      AnimationParametersBlock mAnimationParametersBlock;
      SharedPtr<Vk::Texture> mWindmapTexture;
   };
}
