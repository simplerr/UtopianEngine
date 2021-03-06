#pragma once
#include <glm/glm.hpp>
#include <array>
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/ShaderBuffer.h"
#include "vulkan/Texture.h"
#include "utility/Common.h"
#include "utility/math/Ray.h"
#include "imgui\imgui.h"

namespace Utopian
{
   struct TerrainMaterial
   {
      SharedPtr<Vk::Texture> diffuse;
      SharedPtr<Vk::Texture> normal;
      SharedPtr<Vk::Texture> displacement;
   };

   class Terrain
   {
   public:

      #define MAP_RESOLUTION 512

      struct TerrainDebugDescriptorSets
      {
         ImTextureID heightmap;
         ImTextureID normalmap;
         ImTextureID blendmap;
      };

      UNIFORM_BLOCK_BEGIN(BrushBlock)
         UNIFORM_PARAM(glm::vec2, brushPos)
         UNIFORM_PARAM(float, radius)
         UNIFORM_PARAM(float, strength)
         UNIFORM_PARAM(int, mode) // 0 = height, 1 = blend
         UNIFORM_PARAM(int, operation) // 0 = add, 1 = remove
         UNIFORM_PARAM(int, blendLayer)
      UNIFORM_BLOCK_END()

      UNIFORM_BLOCK_BEGIN(SettingsBlock)
         UNIFORM_PARAM(float, amplitudeScaling)
      UNIFORM_BLOCK_END()

      Terrain(Vk::Device* device);
      ~Terrain();

      void Update();

      glm::vec2 TransformToUv(float x, float z);

      void AddMaterial(std::string name, std::string diffuse, std::string normal, std::string displacement);

      void SaveHeightmap(std::string filename);
      void SaveBlendmap(std::string filename);
      void LoadHeightmap(std::string filename);
      void LoadBlendmap(std::string filename);

      glm::vec3 GetIntersectPoint(Ray ray);
      SharedPtr<Vk::Image>& GetHeightmapImage();
      SharedPtr<Vk::Image>& GetNormalmapImage();
      SharedPtr<Vk::Image>& GetBlendmapImage();
      Vk::Mesh* GetMesh();
      TerrainMaterial GetMaterial(std::string material);

      float GetAmplitudeScaling();
      float GetHeight(float x, float z);
      glm::vec3 GetNormal(float x, float z);

      uint32_t GetMapResolution();
      float GetTerrainSize();

      void SetBrushBlock(const SharedPtr<BrushBlock> brushBlock);
      SharedPtr<BrushBlock> GetBrushBlock();

      void SetAmplitudeScaling(float amplitudeScaling);

      void RenderNormalmap();
      void RenderBlendmap();
      void RetrieveHeightmap();
      void UpdatePhysicsHeightmap();
   private:
      void EffectRecomiledCallback(std::string name);
      void GeneratePatches(float cellSize, int numCells);
      void GenerateTerrainMaps();
      void SetupHeightmapEffect();
      void SetupNormalmapEffect();
      void SetupBlendmapEffect();
      void RenderHeightmap();
      Ray LinearSearch(Ray ray);

   private:
      Vk::Device* mDevice;
      Vk::StaticModel* mQuadModel;
      float mAmplitudeScaling = 50;
      SettingsBlock settingsBlock;
      SharedPtr<BrushBlock> mBrushBlock;

      // Height & normal map members
      SharedPtr<Vk::Effect> mHeightmapEffect;
      SharedPtr<Vk::Image> heightmapImage;
      SharedPtr<Vk::RenderTarget> heightmapRenderTarget;

      SharedPtr<Vk::Effect> mNormalmapEffect;
      SharedPtr<Vk::Image> normalImage;
      SharedPtr<Vk::RenderTarget> normalRenderTarget;

      SharedPtr<Vk::Effect> mBlendmapEffect;
      SharedPtr<Vk::Image> blendmapImage;
      SharedPtr<Vk::RenderTarget> blendmapRenderTarget;

      // Heightmap on CPU
      SharedPtr<Vk::Image> hostImage;
      std::array<float, MAP_RESOLUTION * MAP_RESOLUTION> heightmap;
      float terrainSize;

      std::map<std::string, TerrainMaterial> mMaterials;

      SharedPtr<Vk::Sampler> sampler;
      TerrainDebugDescriptorSets mDebugDescriptorSets;
   };
}
