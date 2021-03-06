#pragma once
#include <glm/glm.hpp>
#include "core/SceneNode.h"
#include "core/components/Component.h"
#include "core/LightData.h"
#include "utility/Common.h"

namespace Utopian
{
   class Actor;

   class Light : public SceneNode
   {
   public:
      Light();
      ~Light();

      void Initialize();
      void OnDestroyed();

      static SharedPtr<Light> Create();

      void SetLightData(const Utopian::LightData& lightData);
      void SetMaterials(const glm::vec4& ambient, const glm::vec4& diffuse, const glm::vec4& specular);
      void SetMaterial(const glm::vec4& color);
      void SetMaterial(const Utopian::Material & material);

      void SetDirection(const glm::vec3& direction);
      void SetRange(float range);
      void SetSpot(float spot);
      void SetAtt(float a0, float a1, float a2);
      void SetType(Utopian::LightType type);
      void SetIntensity(float ambient, float diffuse, float specular);

      // Getters
      const Utopian::LightData& GetLightData();
      const glm::vec3& GetDirection() const;
      const glm::vec3& GetAtt() const;
      const glm::vec3& GetIntensity() const;
      Utopian::LightData* GetLightDataPtr();
      const Utopian::LightData& GetLightData() const;
      Utopian::Material GetMaterial() const;
      float GetRange() const;
      float GetSpot() const;
      int GetType() const;

   private:
      Utopian::LightData mLightData;
   };
}
