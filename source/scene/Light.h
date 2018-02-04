#pragma once
#include <glm/glm.hpp>
#include "scene/SceneNode.h"
#include "scene/SceneComponent.h"
#include "LightData.h"
#include "utility/Common.h"

namespace Scene
{
	class Actor;

	class Light : public SceneNode
	{
	public:
		Light();
		~Light();

		void Initialize();

		static SharedPtr<Light> Create();

		void SetLightData(const Vulkan::LightData& lightData);
		void SetMaterials(const const vec4& ambient, const vec4& diffuse, const vec4& specular);
		void SetMaterial(const vec4& color);
		void SetMaterial(const Vulkan::Material & material);

		void SetDirection(const vec3& direction);
		void SetRange(float range);
		void SetSpot(float spot);
		void SetAtt(float a0, float a1, float a2);
		void SetType(Vulkan::LightType type);
		void SetIntensity(float ambient, float diffuse, float specular);

		// Getters
		const Vulkan::LightData& GetLightData();
		const vec3& GetDirection() const;
		const vec3& GetAtt() const;
		const vec3& GetIntensity() const;
		Vulkan::LightData* GetLightDataPtr();
		const Vulkan::LightData& GetLightData() const;
		Vulkan::Material GetMaterial() const;
		float	 GetRange() const;
		float	 GetSpot() const;
		int		 GetType() const;

	private:
		Vulkan::LightData mLightData;
	};
}
