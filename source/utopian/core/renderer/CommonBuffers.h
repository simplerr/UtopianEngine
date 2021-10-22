#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"
#include "core/LightData.h"

namespace Utopian
{
   class LightUniformBuffer : public Utopian::Vk::ShaderBuffer
   {
   public:
      virtual void UpdateMemory()
      {
         // Map and update the light data
         uint8_t* mapped;
         uint32_t dataOffset = 0;
         uint32_t dataSize = sizeof(constants);
         mBuffer->MapMemory((void**)&mapped);
         memcpy(mapped, &constants.numLights, dataSize);
         mBuffer->UnmapMemory();

         // Map and update number of lights
         dataOffset += dataSize;
         dataSize = (uint32_t)lights.size() * sizeof(Utopian::LightData);
         mBuffer->MapMemory((void**)&mapped);
         memcpy(&mapped[dataOffset], lights.data(), dataSize);
         mBuffer->UnmapMemory();
      }

      virtual int GetSize()
      {
         return (NUM_MAX_LIGHTS) * sizeof(Utopian::LightData) + sizeof(constants);
      }

      virtual std::string GetDebugName()
      {
         return "LightUniformBuffer";
      }

      struct {
         float numLights;
         glm::vec3 garbage;
      } constants;

      const uint32_t NUM_MAX_LIGHTS = 100;
      std::vector<Utopian::LightData> lights;
   };

   class SettingsUniformBuffer : public Utopian::Vk::ShaderBuffer
   {
   public:
      virtual void UpdateMemory()
      {
         // Map uniform buffer and update it
         uint8_t *mapped;
         mBuffer->MapMemory((void**)&mapped);
         memcpy(mapped, &data, sizeof(data));
         mBuffer->UnmapMemory();
      }

      virtual std::string GetDebugName()
      {
         return "SettingsUniformBuffer";
      }

      virtual int GetSize()
      {
         return sizeof(data);
      }

      struct {
         glm::vec3 fogColor;
         float fogStart;
         float fogDistance;
         int cascadeColorDebug;
         int useIBL;
      } data;
   };

   UNIFORM_BLOCK_BEGIN(CascadeBlock)
      UNIFORM_PARAM(float, cascadeSplits[4])
      UNIFORM_PARAM(glm::mat4, cascadeViewProjMat[4])
      UNIFORM_PARAM(glm::mat4, cameraViewMat)
      UNIFORM_PARAM(int, shadowSampleSize)
      UNIFORM_PARAM(int, shadowsEnabled)
   UNIFORM_BLOCK_END()
}
