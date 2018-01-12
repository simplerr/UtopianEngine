#pragma once
#include "scene/SceneComponent.h"
#include "scene/Light.h"
#include "LightData.h"
#include "utility/Common.h"

namespace Scene
{
	class SceneEntity;

	class CLight : public SceneComponent
	{
	public:
		CLight(SceneEntity* parent);
		~CLight();

		void Update() override;
		void OnCreated() override;

		// Setters
		void SetMaterials(const const vec4& ambient, const vec4& diffuse, const vec4& specular);
		void SetMaterial(const vec4& color);
		void SetDirection(const vec3& direction);
		void SetDirection(float x, float y, float z);
		void SetRange(float range);
		void SetSpot(float spot);
		void SetAtt(float a0, float a1, float a2);
		void SetType(Vulkan::LightType type);
		void SetIntensity(float ambient, float diffuse, float specular);

		// Getters
		const vec3& GetDirection() const;
		const vec3& GetAtt() const;
		const vec3& GetIntensity() const;
		Vulkan::Material GetMaterial() const;
		float	 GetRange() const;
		float	 GetSpot() const;
		int		 GetType() const;

		// Type identification
		static uint32_t GetStaticType() {
			return SceneComponent::ComponentType::LIGHT;
		}

		virtual uint32_t GetType() {
			return GetStaticType();
		}

	private:
		SharedPtr<Light> mInternal;
	};
}
