#pragma once
#include "core/components/Component.h"
#include "core/renderer/Light.h"
#include "LightData.h"
#include "utility/Common.h"
#include "core/LuaManager.h"

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
		void OnDestroyed() override;
		void PostInit() override;

		LuaPlus::LuaObject GetLuaObject() override;

		// Setters
		void SetMaterials(const glm::vec4& ambient, const glm::vec4& diffuse, const glm::vec4& specular);
		void SetMaterial(const glm::vec4& color);
		void SetMaterial(const Utopian::Material& material);

		void SetDirection(const glm::vec3& direction);
		void SetDirection(float x, float y, float z);
		void SetRange(float range);
		void SetSpot(float spot);
		void SetAtt(float a0, float a1, float a2);
		void SetAttenuation(glm::vec3 attenuation);
		void SetType(Utopian::LightType type);
		void SetIntensity(float ambient, float diffuse, float specular);
		void SetIntensity(glm::vec3 intensity);


		// Getters
		const Utopian::LightData& GetLightData() const;
		const glm::vec3& GetDirection() const;
		const glm::vec3& GetAtt() const;
		const glm::vec3& GetIntensity() const;
		Utopian::Material GetMaterial() const;
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
