#pragma once
#include <glm/glm.hpp>
#include "core/components/Component.h"
#include "core/Transform.h"
#include "vulkan/VulkanInclude.h"
#include "utility/Common.h"

#include "BulletDynamics/Dynamics/btRigidBody.h"

class btRigidBody;
class btCollisionShape;

namespace Utopian
{
	class Actor;
	class CTransform;
	class CRenderable;

	class CRigidBody : public Component
	{
	public:
		CRigidBody(Actor* parent);
		~CRigidBody();

		void Update() override;
		void OnCreated() override;
		void OnDestroyed() override;
		void PostInit() override;

		void Activate();
		void Deactivate();
		bool IsActiveted();

		void SetPosition(const glm::vec3& position);
		void SetRotation(const glm::vec3& rotation);
		void SetQuaternion(const glm::quat& quaternion);

		float GetMass() const;
		float GetFriction() const;
		float GetRollingFriction() const;
		float GetRestitution() const;

		void SetMass(float mass);
		void SetFriction(float friction);
		void SetRollingFriction(float rollingFriction);
		void SetRestitution(float restitution);

		const Transform& GetTransform() const;

		LuaPlus::LuaObject GetLuaObject() override;

		// Type identification
		static uint32_t GetStaticType() {
			return Component::ComponentType::RIGID_BODY;
		}

		virtual uint32_t GetType() {
			return GetStaticType();
		}

	private:
		CTransform* mTransform;
		CRenderable* mRenderable;
		btRigidBody* mRigidBody;
		btCollisionShape* mCollisionShape;

		float mMass;
		float mFriction;
		float mRollingFriction;
		float mRestitution;
	};

	class MotionState : public btMotionState
	{
	public:
		MotionState(CRigidBody* rigidBody);

		// Engine -> Bullet
		void getWorldTransform(btTransform& worldTrans) const override;

		// Bullet -> Engine
		void setWorldTransform(const btTransform& worldTrans) override;

	private:
		CRigidBody* mRigidBody;
	};
}