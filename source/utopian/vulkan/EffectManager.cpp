#include <sys/stat.h>
#include "vulkan/EffectManager.h"
#include "vulkan/Effect.h"
#include "core/Log.h"
#include "imgui/imgui.h"

namespace Utopian::Vk
{
   EffectManager::EffectManager()
   {
   }

   EffectManager::~EffectManager()
   {
   }

   EffectManager& gEffectManager()
   {
      return EffectManager::Instance();
   }

   void EffectManager::Update(double deltaTime)
   {
   }

   void EffectManager::RecompileModifiedShaders()
   {
      for (auto& trackedEffect : mEffects)
      {
         ShaderCreateInfo shaderCreateInfo = trackedEffect.effect->GetShaderCreateInfo();
         time_t updatedModificationTime = GetLatestModification(shaderCreateInfo);
         if (updatedModificationTime > trackedEffect.lastModification)
         {
            if (trackedEffect.effect->RecompileShader())
               UTO_LOG("Recompiled \"" + shaderCreateInfo.fragmentShaderPath + "\" succesfully");
            else
               UTO_LOG("Recompilation of \"" + shaderCreateInfo.fragmentShaderPath + "\" failed");

            NotifyCallbacks(trackedEffect.effect->GetVertexShaderPath());
            trackedEffect.lastModification = updatedModificationTime;
         }
      }
   }
   
   void EffectManager::RecompileAllShaders()
   {
      for (auto& trackedEffect : mEffects)
      {
         ShaderCreateInfo shaderCreateInfo = trackedEffect.effect->GetShaderCreateInfo();
         time_t updatedModificationTime = GetLatestModification(shaderCreateInfo);
         trackedEffect.effect->RecompileShader();
         NotifyCallbacks(trackedEffect.effect->GetVertexShaderPath());
         trackedEffect.lastModification = updatedModificationTime;
      }

      UTO_LOG("Recompiled all shaders");
   }

   void EffectManager::NotifyCallbacks(std::string name)
   {
      for (auto& callback : mRecompileCallbacks)
      {
         callback(name);
      }
   }

   uint64_t EffectManager::GetLatestModification(const ShaderCreateInfo& shaderCreateInfo)
   {
      time_t latest = 0;
      struct stat file_details;

      if (stat(shaderCreateInfo.vertexShaderPath.c_str(), &file_details) == 0)
         latest = glm::max(latest, file_details.st_mtime);

      if (stat(shaderCreateInfo.fragmentShaderPath.c_str(), &file_details) == 0)
         latest = glm::max(latest, file_details.st_mtime);

      if (stat(shaderCreateInfo.geometryShaderPath.c_str(), &file_details) == 0)
         latest = glm::max(latest, file_details.st_mtime);

      if (stat(shaderCreateInfo.tescShaderPath.c_str(), &file_details) == 0)
         latest = glm::max(latest, file_details.st_mtime);

      if (stat(shaderCreateInfo.teseShaderPath.c_str(), &file_details) == 0)
         latest = glm::max(latest, file_details.st_mtime);

      if (stat(shaderCreateInfo.computeShaderPath.c_str(), &file_details) == 0)
         latest = glm::max(latest, file_details.st_mtime);

      //printf("Last modified time: %s", ctime(&latest));
      return latest;
   }
}