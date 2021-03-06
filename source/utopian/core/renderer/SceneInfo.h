#pragma once
#include <glm/glm.hpp>
#include <array>
#include "vulkan/VulkanApp.h"
#include "core/renderer/Renderable.h"
#include "core/renderer/Light.h"
#include "core/Terrain.h"

#define SHADOW_MAP_CASCADE_COUNT 4

namespace Utopian
{
   class Terrain;

   struct InstanceDataGPU
   {
      glm::mat4 world;
   };

   struct InstanceData
   {
      InstanceData(glm::vec3 _position, glm::vec3 _rotation, glm::vec3 _scale)
      : position(_position), rotation(_rotation), scale(_scale) {}

      glm::vec3 position;
      glm::vec3 rotation;
      glm::vec3 scale;
   };

   class InstanceGroup
   {
   public:
      InstanceGroup(uint32_t assetId, bool animated = false, bool castShadows = false);
      ~InstanceGroup();

      void AddInstance(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
      void RemoveInstances();
      void RemoveInstancesWithinRadius(glm::vec3 position, float radius);
      void UpdateAltitudes(const SharedPtr<Terrain>& terrain);
      void BuildBuffer(Vk::Device* device);
      void SetAnimated(bool animated);
      void SetCastShadows(bool castShadows);
      void SaveToFile(std::ofstream& fout);

      uint32_t GetAssetId();
      uint32_t GetNumInstances();
      Vk::Buffer* GetBuffer();
      Vk::StaticModel* GetModel();
      bool IsAnimated();
      bool IsCastingShadows();

   private:
      SharedPtr<Vk::Buffer> mInstanceBuffer;
      SharedPtr<Vk::StaticModel> mModel;
      std::vector<InstanceDataGPU> mInstances; // Uploaded to GPU
      std::vector<InstanceData> mInstanceData;
      uint32_t mAssetId;
      bool mAnimated;
      bool mCastShadows;
   };

   class Cascade
   {
   public:
      float splitDepth;
      glm::mat4 viewProjMatrix;
   };

   // This uniform buffer contains data that is common in multiple shaders.
   // It is defined in data\shaders\include\shared_variables.glsl and expected to be included
   // in all shaders that needs any of its data, meant to act like built-in shader variables.
   UNIFORM_BLOCK_BEGIN(SharedShaderVariables)
      UNIFORM_PARAM(glm::mat4, viewMatrix)
      UNIFORM_PARAM(glm::mat4, projectionMatrix)
      UNIFORM_PARAM(glm::mat4, inverseProjectionMatrix)
      UNIFORM_PARAM(glm::vec4, eyePos)
      UNIFORM_PARAM(glm::vec2, viewportSize)
      UNIFORM_PARAM(glm::vec2, mouseUV)
      UNIFORM_PARAM(float, time)
   UNIFORM_BLOCK_END()

   struct SceneInfo
   {
      std::vector<Renderable*> renderables;
      std::vector<Light*> lights;
      std::vector<Camera*> cameras;
      SharedPtr<Terrain> terrain;
      std::vector<SharedPtr<InstanceGroup>> instanceGroups;
      std::array<Cascade, SHADOW_MAP_CASCADE_COUNT> cascades;
      SharedPtr<Vk::Buffer> im3dVertices;

      // The light that will cast shadows
      // Currently assumes that there only is one directional light in the scene
      Light* directionalLight;

      // Todo: Remove these
      glm::mat4 viewMatrix;
      glm::mat4 projectionMatrix;
      glm::vec3 eyePos;
      SharedShaderVariables sharedVariables;

      struct SunInfo
      {
         float azimuth = 0.0f;
         glm::vec3 direction;
      } sunInfo;
   };
}
