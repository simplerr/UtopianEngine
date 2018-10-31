#pragma once
#include "core/components/Component.h"
#include "core/renderer/Light.h"
#include "LightData.h"
#include "utility/Common.h"

namespace Utopian
{
	class Actor;

	class CLight : public Component
	{
	public:
		CLight(Actor* parent);
		~CLight();

		void Update() override;
		void OnCreated() override;

		// Setters
		void SetMaterials(const const vec4& ambient, const vec4& diffuse, const vec4& specular);
		void SetMaterial(const vec4& color);
		void SetMaterial(const Utopian::Vk::Material& material);

		void SetDirection(const vec3& direction);
		void SetDirection(float x, float y, float z);
		void SetRange(float range);
		void SetSpot(float spot);
		void SetAtt(float a0, float a1, float a2);
		void SetAttenuation(vec3 attenuation);
		void SetType(Utopian::Vk::LightType type);
		void SetIntensity(float ambient, float diffuse, float specular);
		void SetIntensity(vec3 intensity);


		// Getters
		const Utopian::Vk::LightData& GetLightData() const;
		const vec3& GetDirection() const;
		const vec3& GetAtt() const;
		const vec3& GetIntensity() const;
		Utopian::Vk::Material GetMaterial() const;
		float	 GetRange() const;
		float	 GetSpot() const;
		int		 GetLightType() const;

		// Type identification
		static uint32_t GetStaticType() {
			return Component::ComponentType::LIGHT;
		}

		virtual uint32_t GetType() {
			return GetStaticType();
		}

	private:
		SharedPtr<Light> mInternal;
	};
}
