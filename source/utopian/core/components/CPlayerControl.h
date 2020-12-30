#pragma once
#include <core/components/CRigidBody.h>
#include <glm/glm.hpp>
#include "core/components/Component.h"
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"
#include "core/LuaManager.h"

namespace Utopian
{
	class Actor;
	class CCamera;
	class CTransform;
	class COrbit;
	class CNoClip;
	class CRigidBody;

	class CPlayerControl : public Component
	{
	public:
		CPlayerControl(Actor* parent);
		~CPlayerControl();

		void Update() override;
		void PostInit() override;

		LuaPlus::LuaObject GetLuaObject() override;
		
		void SetSpeed(float speed);
		void SetJumpStrength(float jumpStrength);

		float GetSpeed() const;
		float GetJumpStrength() const;

		// Type identification
		static uint32_t GetStaticType() {
			return Component::ComponentType::PLAYER_CONTROL;
		}

		virtual uint32_t GetType() {
			return GetStaticType();
		}

	private:
		CCamera* mCamera; // For convenience
		CNoClip* mNoClip;
		COrbit* mOrbit;
		CTransform* mTransform;
		CRigidBody* mRigidBody;
		float mSpeed = 5.0f;
		float mJumpStrength = 5.0f;
	};
}
