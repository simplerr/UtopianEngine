#pragma once
#include <glm/glm.hpp>
#include "Component.h"
#include "core/Transform.h"
#include "core/LuaManager.h"

using namespace glm;

namespace Utopian
{
	class Actor;

	class CTransform : public Component
	{
	public:
		CTransform(Actor* parent, const vec3& position);
		~CTransform();

		void PostInit() override;

		LuaPlus::LuaObject GetLuaObject() override;

		void SetTransform(const Transform& transform);
		void SetPosition(const vec3& position);
		void SetRotation(const vec3& rotation);
		void SetScale(const vec3& scale);

		void AddTranslation(const vec3& translation);

		void AddRotation(float x, float y, float z);
		void AddRotation(const vec3& rotation);
		void AddScale(float x, float y, float z);
		void AddScale(const vec3& scale);

		const Transform& GetTransform() const;
		const vec3& GetPosition() const;
		const vec3& GetRotation() const;
		const vec3& GetScale() const;
		const mat4& GetWorldMatrix() const;
		const mat4& GetWorldInverseTransposeMatrix() const;

		// Type identification
		static uint32_t GetStaticType() {
			return Component::ComponentType::TRANSFORM;
		}

		virtual uint32_t GetType() {
			return GetStaticType();
		}

	private:
		Transform mTransform;
	};
}