#pragma once
#include <glm/glm.hpp>
#include <string>

namespace Utopian
{
   const std::string SKY_DOME = "dome";
   const std::string SKY_BOX = "box";
   const std::string SKY_ATMOSPHERE = "atmosphere";

   enum ShadingMethod {PHONG, PBR};

   struct RenderingSettings
   {
      ShadingMethod shadingMethod = ShadingMethod::PHONG;
      std::string sky = SKY_DOME;
      glm::vec4 fogColor = glm::vec4(113 / 255.0f, 129 / 255.0f, 232 / 255.0f, 1.0f);
      bool deferredPipeline = true;
      float fogStart = 200.0f;
      float fogDistance = 140.0f;
      float ssaoRadius = 6.0f;
      float ssaoBias = 0.0f;
      int blurRadius = 2;
      float grassViewDistance = 0 * 1800.0f;
      int blockViewDistance = 2;
      bool shadowsEnabled = true;
      bool normalMapping = true;
      bool ssaoEnabled = false;
      bool ssrEnabled = true;
      bool iblEnabled = true;
      bool bloomEnabled = false;
      bool skyboxReflections = true;
      bool waterEnabled = true;
      bool terrainEnabled = false;
      bool fxaaEnabled = true;
      bool fxaaDebug = false;
      bool godRaysEnabled = true;
      float fxaaThreshold = 0.125;
      int shadowSampleSize = 1;
      bool cascadeColorDebug = 0;
      float cascadeSplitLambda = 0.927f;
      float sunSpeed = 0.0f;
      float sunInclination = -35.0f;
      float sunAzimuth = 0.0f;
      float tessellationFactor = 2.8f;
      float terrainTextureScaling = 200.0f;
      float terrainBumpmapAmplitude = 0.08f;
      bool terrainWireframe = 0;
      int tonemapping = 2; // 0 = Reinhard, 1 = Uncharted 2, 2 = Exposure, 3 = None
      float exposure = 1.4f;
      float bloomThreshold = 1.5f;
      float windStrength = 5.0f;
      float windFrequency = 10000.0f;
      bool windEnabled = true;

      // Water
      int numWaterCells = 512;
      float waterLevel = 0.0f;
      glm::vec3 waterColor = glm::vec3(0.0f, 0.0f, 0.0f);
      glm::vec3 foamColor = glm::vec3(0.9f);
      float waveSpeed = 1.0f;
      float foamSpeed = 4.0f;
      float waterDistortionStrength = 0.02f;
      float shorelineDepth = 1.2f;
      float waveFrequency = 0.3f;
      float waterSpecularity = 128.0f;
      float waterTransparency = 0.0f;
      float underwaterViewDistance = 8.0f;

      float outlineWidth = 1.0f;
   };
}
