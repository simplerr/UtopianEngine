#pragma once
#include <glm/glm.hpp>
#include "core/SceneNode.h"
#include "core/components/Component.h"
#include "LightData.h"
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

		static SharedPtr<Light> Create();

		void SetLightData(const Utopian::Vk::LightData& lightData);
		void SetMaterials(const const vec4& ambient, const vec4& diffuse, const vec4& specular);
		void SetMaterial(const vec4& color);
		void SetMaterial(const Utopian::Vk::Material & material);

		void SetDirection(const vec3& direction);
		void SetRange(float range);
		void SetSpot(float spot);
		void SetAtt(float a0, float a1, float a2);
		void SetType(Utopian::Vk::LightType type);
		void SetIntensity(float ambient, float diffuse, float specular);

		// Getters
		const Utopian::Vk::LightData& GetLightData();
		const vec3& GetDirection() const;
		const vec3& GetAtt() const;
		const vec3& GetIntensity() const;
		Utopian::Vk::LightData* GetLightDataPtr();
		const Utopian::Vk::LightData& GetLightData() const;
		Utopian::Vk::Material GetMaterial() const;
		float	 GetRange() const;
		float	 GetSpot() const;
		int		 GetType() const;

	private:
		Utopian::Vk::LightData mLightData;
	};
}
