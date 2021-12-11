#include "core/renderer/RenderSettings.h"
#include "core/Engine.h"
#include "core/Terrain.h"
#include "core/renderer/ImGuiRenderer.h"
#include "core/LuaManager.h"

namespace Utopian
{
   void DisplayRenderSettings(RenderingSettings& renderSettings, Terrain* terrain)
   {
      ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

      if (ImGui::CollapsingHeader("Features", ImGuiTreeNodeFlags_DefaultOpen))
      {
         static int shadingMethod = renderSettings.shadingMethod;
         if (ImGui::Combo("Shading method", &shadingMethod, "Phong\0PBR\0"))
         {
            renderSettings.shadingMethod = (ShadingMethod)shadingMethod;
         }

         ImGui::Checkbox("Shadows", &renderSettings.shadowsEnabled);
         ImGui::Checkbox("Normal mapping", &renderSettings.normalMapping);
         ImGui::Checkbox("SSAO", &renderSettings.ssaoEnabled);
         ImGui::Checkbox("SSR", &renderSettings.ssrEnabled);
         ImGui::Checkbox("IBL", &renderSettings.iblEnabled);
         ImGui::Checkbox("Bloom", &renderSettings.bloomEnabled);
         ImGui::Checkbox("Skybox reflections", &renderSettings.skyboxReflections);
         ImGui::Checkbox("Water", &renderSettings.waterEnabled);
         ImGui::Checkbox("God rays", &renderSettings.godRaysEnabled);
         ImGui::Checkbox("FXAA", &renderSettings.fxaaEnabled);
         ImGui::Checkbox("FXAA debug", &renderSettings.fxaaDebug);
         ImGui::Checkbox("Cascade color debug", &renderSettings.cascadeColorDebug);
         ImGui::Checkbox("Terrain wireframe", &renderSettings.terrainWireframe);
         ImGui::Checkbox("Wind enabled", &renderSettings.windEnabled);
      }

      if (ImGui::CollapsingHeader("Depth of Field settings"))
      {
         ImGui::Checkbox("DOF enabled", &renderSettings.dofEnabled);
         ImGui::SliderFloat("DOF start", &renderSettings.dofStart, 0.0f, 50.0f);
         ImGui::SliderFloat("DOF range", &renderSettings.dofRange, 0.0f, 50.0f);
      }

      if (ImGui::CollapsingHeader("Fog settings"))
      {
         ImGui::ColorEdit4("Fog color", &renderSettings.fogColor.x);
         ImGui::SliderFloat("Fog start", &renderSettings.fogStart, 0.0f, 800.0f);
         ImGui::SliderFloat("Fog distance", &renderSettings.fogDistance, 0.0f, 800.0f);
      }

      //ImGui::SliderFloat("SSAO radius", &renderSettings.ssaoRadius, 0.0f, 20.0f);
      //ImGui::SliderFloat("SSAO bias", &renderSettings.ssaoBias, 0.0f, 10.0f);
      //ImGui::SliderInt("SSAO blur radius", &renderSettings.blurRadius, 1, 20);
      //ImGui::SliderInt("Block view distance", &renderSettings.blockViewDistance, 1, 10);
      //ImGui::SliderFloat("Grass view distance", &renderSettings.grassViewDistance, 0.0f, 10000.0f);

      if (ImGui::CollapsingHeader("Mixed settings"))
      {
         ImGui::SliderFloat("Ambient intensity", &renderSettings.ambientIntensity, 0.0f, 1.0f);
         ImGui::SliderFloat("FXAA threshold", &renderSettings.fxaaThreshold, 0.0f, 1.5f);
         ImGui::SliderFloat("SSAO radius", &renderSettings.ssaoRadius, 0.0f, 0.15f);
         ImGui::SliderInt("Shadow sample size", &renderSettings.shadowSampleSize, 0, 10);
         ImGui::SliderFloat("Cascade split lambda", &renderSettings.cascadeSplitLambda, 0.0f, 1.0f);
         ImGui::SliderFloat("Sun inclination", &renderSettings.sunInclination, -90.0f, 90.0f);
         //ImGui::SliderFloat("Sun azimuth", &renderSettings.sunAzimuth, -180.0f, 180.0f);
         ImGui::SliderFloat("Sun speed", &renderSettings.sunSpeed, 0.0f, 10.0f);
         ImGui::SliderFloat("Exposure", &renderSettings.exposure, 0.0f, 5.0f);
         ImGui::Combo("Tonemapping", &renderSettings.tonemapping, "Reinhard\0Uncharted 2\0Exposure\0None\0ACES\0");
         ImGui::SliderFloat("Bloom threshold", &renderSettings.bloomThreshold, 0.5f, 10.0f);
         ImGui::SliderFloat("Wind strength", &renderSettings.windStrength, 0.0f, 25.0f);
         ImGui::SliderFloat("Wind frequency", &renderSettings.windFrequency, 0.0f, 30000.0f);
         ImGui::SliderFloat("Outline width", &renderSettings.outlineWidth, 1.0f, 20.0f);
      }

      if (ImGui::CollapsingHeader("Terrain settings"))
      {
         ImGui::SliderFloat("Tessellation factor", &renderSettings.tessellationFactor, 0.0f, 5.0f);

         float amplitudeScaling = terrain->GetAmplitudeScaling();
         if (ImGui::SliderFloat("Terrain amplitude", &amplitudeScaling, 0.4f, 100))
         {
            terrain->SetAmplitudeScaling(amplitudeScaling);
            //UpdateInstanceAltitudes(); Can't do this because the buffer is in use by a command buffer
         }

         ImGui::SliderFloat("Terrain texture scaling", &renderSettings.terrainTextureScaling, 1.0f, 600.0f);
         ImGui::SliderFloat("Terrain bumpmap amplitude", &renderSettings.terrainBumpmapAmplitude, 0.0078f, 0.4f);
      }

      if (ImGui::CollapsingHeader("Water settings"))
      {
         ImGui::SliderFloat("Water level", &renderSettings.waterLevel, -15.0f, 15.0f);
         ImGui::ColorEdit3("Water color", &renderSettings.waterColor.x);
         ImGui::ColorEdit3("Foam color", &renderSettings.foamColor.x);
         ImGui::SliderFloat("Wave speed", &renderSettings.waveSpeed, 0.1f, 10.0f);
         ImGui::SliderFloat("Foam speed", &renderSettings.foamSpeed, 0.1f, 10.0f);
         ImGui::SliderFloat("Distortion strength", &renderSettings.waterDistortionStrength, 0.005f, 0.1f);
         ImGui::SliderFloat("Shoreline depth", &renderSettings.shorelineDepth, 0.0f, 8.0f);
         ImGui::SliderFloat("Wave frequency", &renderSettings.waveFrequency, 0.0f, 5.0f);
         ImGui::SliderFloat("Water specularity", &renderSettings.waterSpecularity, 1.0f, 1024.0f);
         ImGui::SliderFloat("Water transparency", &renderSettings.waterTransparency, 0.0f, 1.0f);
         ImGui::SliderFloat("Underwater view distance", &renderSettings.underwaterViewDistance, 0.0f, 40.0f);
      }
   }

   void LoadSettingsFromFile(RenderingSettings& renderSettings, Engine* engine, std::string fileName)
   {
      gLuaManager().ExecuteFile(fileName.c_str());

      LuaPlus::LuaObject luaSettings = gLuaManager().GetLuaState()->GetGlobal("settings");
      if (luaSettings.IsNil())
         assert(0);

      // Note: the sceneSource configuration should not be handled by DeferredRenderingPlugin
      engine->SetSceneSource(luaSettings["sceneSource"].ToString());

      renderSettings.shadingMethod = ShadingMethod::PHONG;
      std::string shadingMethod = luaSettings["shadingMethod"].ToString();
      if (shadingMethod == "phong")
         renderSettings.shadingMethod = ShadingMethod::PHONG;
      else if (shadingMethod == "pbr")
         renderSettings.shadingMethod = ShadingMethod::PBR;

      renderSettings.sky = luaSettings["sky"].ToString();
      renderSettings.ambientIntensity = (float)luaSettings["ambientIntensity"].ToNumber();
      renderSettings.fogColor = glm::vec4(luaSettings["fogColor_r"].ToNumber(),
                                              luaSettings["fogColor_g"].ToNumber(),
                                              luaSettings["fogColor_b"].ToNumber(), 1.0f);
      renderSettings.deferredPipeline = luaSettings["deferredPipeline"].GetBoolean();
      renderSettings.fogStart = (float)luaSettings["fogStart"].ToNumber();
      renderSettings.fogDistance = (float)luaSettings["fogDistance"].ToNumber();
      renderSettings.ssaoRadius = (float)luaSettings["ssaoRadius"].ToNumber();
      renderSettings.ssaoBias = (float)luaSettings["ssaoBias"].ToNumber();
      renderSettings.blurRadius = (int)luaSettings["blurRadius"].ToInteger();
      renderSettings.grassViewDistance = (float)luaSettings["grassViewDistance"].ToNumber();
      renderSettings.blockViewDistance = (int)luaSettings["blockViewDistance"].ToInteger();
      renderSettings.shadowsEnabled = luaSettings["shadowsEnabled"].GetBoolean();
      renderSettings.normalMapping = luaSettings["normalMapping"].GetBoolean();
      renderSettings.ssaoEnabled = luaSettings["ssaoEnabled"].GetBoolean();
      renderSettings.ssrEnabled = luaSettings["ssrEnabled"].GetBoolean();
      renderSettings.iblEnabled = luaSettings["iblEnabled"].GetBoolean();
      renderSettings.bloomEnabled = luaSettings["bloomEnabled"].GetBoolean();
      renderSettings.skyboxReflections = luaSettings["skyboxReflections"].GetBoolean();
      renderSettings.waterEnabled = luaSettings["waterEnabled"].GetBoolean();
      renderSettings.terrainEnabled = luaSettings["terrainEnabled"].GetBoolean();
      renderSettings.fxaaEnabled = luaSettings["fxaaEnabled"].GetBoolean();
      renderSettings.fxaaDebug = luaSettings["fxaaDebug"].GetBoolean();
      renderSettings.godRaysEnabled = luaSettings["godRaysEnabled"].GetBoolean();
      renderSettings.dofEnabled = luaSettings["dofEnabled"].GetBoolean();
      renderSettings.dofStart = (float)luaSettings["dofStart"].ToNumber();
      renderSettings.dofRange = (float)luaSettings["dofRange"].ToNumber();
      renderSettings.fxaaThreshold = (float)luaSettings["fxaaThreshold"].ToNumber();
      renderSettings.shadowSampleSize = (int)luaSettings["shadowSampleSize"].ToInteger();
      renderSettings.cascadeColorDebug = (float)luaSettings["cascadeColorDebug"].ToNumber();
      renderSettings.cascadeSplitLambda = (float)luaSettings["cascadeSplitLambda"].ToNumber();
      renderSettings.sunSpeed = (float)luaSettings["sunSpeed"].ToNumber();
      renderSettings.sunInclination = (float)luaSettings["sunInclination"].ToNumber();
      renderSettings.sunAzimuth = (float)luaSettings["sunAzimuth"].ToNumber();
      renderSettings.tessellationFactor = (float)luaSettings["tessellationFactor"].ToNumber();
      renderSettings.terrainTextureScaling = (float)luaSettings["terrainTextureScaling"].ToNumber();
      renderSettings.terrainBumpmapAmplitude = (float)luaSettings["terrainBumpmapAmplitude"].ToNumber();
      renderSettings.terrainWireframe = (float)luaSettings["terrainWireframe"].ToNumber();
      renderSettings.tonemapping = (int)luaSettings["tonemapping"].ToInteger();
      renderSettings.exposure = (float)luaSettings["exposure"].ToNumber();
      renderSettings.bloomThreshold = (float)luaSettings["bloomThreshold"].ToNumber();
      renderSettings.windStrength = (float)luaSettings["windStrength"].ToNumber();
      renderSettings.windFrequency = (float)luaSettings["windFrequency"].ToNumber();
      renderSettings.windEnabled = (float)luaSettings["windEnabled"].ToNumber();
      renderSettings.numWaterCells = (int)luaSettings["numWaterCells"].ToInteger();
      renderSettings.waterLevel = (float)luaSettings["waterLevel"].ToNumber();
      renderSettings.waterColor = glm::vec3(luaSettings["waterColor_x"].ToNumber(),
                                                luaSettings["waterColor_y"].ToNumber(),
                                                luaSettings["waterColor_z"].ToNumber());
      renderSettings.foamColor = glm::vec3(luaSettings["foamColor_x"].ToNumber(),
                                               luaSettings["foamColor_y"].ToNumber(),
                                               luaSettings["foamColor_z"].ToNumber());
      renderSettings.waveSpeed = (float)luaSettings["waveSpeed"].ToNumber();
      renderSettings.foamSpeed = (float)luaSettings["foamSpeed"].ToNumber();
      renderSettings.waterDistortionStrength = (float)luaSettings["waterDistortionStrength"].ToNumber();
      renderSettings.shorelineDepth = (float)luaSettings["shorelineDepth"].ToNumber();
      renderSettings.waveFrequency = (float)luaSettings["waveFrequency"].ToNumber();
      renderSettings.waterSpecularity = (float)luaSettings["waterSpecularity"].ToNumber();
      renderSettings.waterTransparency = (float)luaSettings["waterTransparency"].ToNumber();
      renderSettings.underwaterViewDistance = (float)luaSettings["underwaterViewDistance"].ToNumber();
   }
}
