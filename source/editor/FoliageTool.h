#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "utility/Common.h"
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/ShaderBuffer.h"
#include "core/Terrain.h"
#include "imgui/imgui.h"
#include "utility/Timer.h"

namespace Utopian
{
   class Terrain;
   struct BrushSettings;

   struct UiAsset
   {
      UiAsset() {
         assetId = 0;
         animated = true;
         scaleFactor = 1.0f;
      }

      uint32_t assetId;
      ImTextureID previewTextureId;
      float scaleFactor;
      bool animated;
   };

   class FoliageTool
   {
   public:
      FoliageTool(Terrain* terrain, Vk::Device* device);
      ~FoliageTool();

      // Uses brush settings from TerrainTool
      // Note: Todo: Remove this dependency
      void SetBrushSettings(BrushSettings* brushSettings);

      void AddAssetToUi(uint32_t assetId, std::string previewPath, float scaleFactor = 0.008f, bool animated = true);

      void Update();
      void RenderUi();

   private:
      void AddVegetation(uint32_t assetId, glm::vec3 position, bool animated, bool castShadows, float scaleFactor = 1.0f);
   private:
      Vk::Device* mDevice;
      Terrain* mTerrain;
      BrushSettings* mBrushSettings;

      struct VegetationSettings
      {
         bool continuous;
         bool randomRotation;
         bool randomScale;
         bool restrictedDeletion; // Only deletes assets of the same type as the selected one
         float frequency;
         float minScale;
         float maxScale;
         uint32_t assetId;
      } mVegetationSettings;

      std::vector<const char*> mAssetNames;
      UiAsset mSelectedUiAsset;
      std::vector<UiAsset> mUiAssets;
      Timestamp mLastAddTimestamp;
   };
}