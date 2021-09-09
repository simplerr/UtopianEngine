#include "core/components/CRigidBody.h"
#include "core/components/CTransform.h"
#include "core/components/CRenderable.h"
#include "core/components/Actor.h"
#include "core/physics/Physics.h"
#include "core/physics/BulletHelpers.h"
#include "core/renderer/Model.h"
#include "imgui/imgui.h"
#include "im3d/im3d.h"
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#include "btBulletDynamicsCommon.h"
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <glm/gtc/quaternion.hpp>

namespace Utopian
{
   MotionState::MotionState(CRigidBody* rigidBody)
   {
      mRigidBody = rigidBody;
   }

   void MotionState::getWorldTransform(btTransform& worldTrans) const
   {
      glm::vec3 position = mRigidBody->GetTransform().GetPosition();
      glm::quat orientation = mRigidBody->GetTransform().GetOrientation();

      worldTrans.setOrigin(ToBulletVec3(position));
      worldTrans.setRotation(ToBulletQuaternion(orientation));
   }

   void MotionState::setWorldTransform(const btTransform& worldTrans)
   {
      glm::vec3 position = ToVec3(worldTrans.getOrigin());
      glm::quat quaternion = ToQuaternion(worldTrans.getRotation());

      mRigidBody->SetPosition(position);
      mRigidBody->SetQuaternion(quaternion);
   }

   CRigidBody::CRigidBody(Actor* parent, CollisionShapeType collisionShape, float mass, float friction,
                          float rollingFriction, float restitution, bool kinematic, glm::vec3 anisotropicFriction)
      : Component(parent)
   {
      SetName("CRigidBody");

      mCollisionShapeType = collisionShape;
      mMass = mass;
      mFriction = friction;
      mRollingFriction = rollingFriction;
      mRestitution = restitution;
      mIsKinematic = kinematic;
      mAnisotropicFriction = anisotropicFriction;
      mRigidBody = nullptr;
   }

   CRigidBody::CRigidBody(Actor* parent)
      : Component(parent)
   {
      SetName("CRigidBody");

      mCollisionShapeType = CollisionShapeType::BOX;
      mMass = 1.0f;
      mFriction = 0.5f;
      mRollingFriction = 0.0f;
      mAnisotropicFriction = glm::vec3(1.0f);
      mRestitution = 0.0f;
      mIsKinematic = false;
      mRigidBody = nullptr;
   }

   CRigidBody::~CRigidBody()
   {
   }

   void CRigidBody::Update()
   {
      //glm::vec3 position = GetTransform().GetPosition();
      //BoundingBox aabb = mRenderable->GetBoundingBox();
      //Im3d::DrawAlignedBox(aabb.GetMin(), aabb.GetMax());
      //Im3d::DrawPoint(position, 20.0f, Im3d::Color_Red);
   }

   void CRigidBody::OnCreated()
   {
      
   }

   void CRigidBody::OnDestroyed()
   {
      RemoveFromWorld();
   }

   void CRigidBody::PostInit()
   {
      mTransform = GetParent()->GetComponent<CTransform>();
      mRenderable = GetParent()->GetComponent<CRenderable>();

      AddToWorld();
   }

   void CRigidBody::AddToWorld()
   {
      RemoveFromWorld();

      MotionState* motionState = new MotionState(this);

      // Zero mass when kinematic
      float mass = mMass;
      if (IsKinematic())
         mass = 0.0f;

      btVector3 localInertia(0, 0, 0);
      BoundingBox aabb = mRenderable->GetBoundingBox();

      if (mCollisionShapeType == CollisionShapeType::BOX)
      {
         mCollisionShape = new btBoxShape(btVector3(aabb.GetWidth() / 2.0f, aabb.GetHeight() / 2.0f, aabb.GetDepth() / 2.0f));
         mCollisionShape->calculateLocalInertia(mass, localInertia);
      }
      else if (mCollisionShapeType == CollisionShapeType::SPHERE)
      {
         mCollisionShape = new btSphereShape(aabb.GetWidth() / 2.0f);
         mCollisionShape->calculateLocalInertia(mass, localInertia);
      }
      else if (mCollisionShapeType == CollisionShapeType::CAPSULE)
      {
         mCollisionShape = new btCapsuleShape(0.25f, 1.0f);
         mCollisionShape->calculateLocalInertia(mass, localInertia);
      }
      else if (mCollisionShapeType == CollisionShapeType::MESH)
      {
         Primitive* primitive = mRenderable->GetInternal()->GetModel()->GetFirstPrimitive();
         btTriangleMesh* triangleMesh = new btTriangleMesh();

         for (uint32_t i = 0; i < primitive->GetNumIndices() - 3; i += 3)
         {
            btVector3 v1 = ToBulletVec3(primitive->vertices[primitive->indices[i]].pos);
            btVector3 v2 = ToBulletVec3(primitive->vertices[primitive->indices[i+1]].pos);
            btVector3 v3 = ToBulletVec3(primitive->vertices[primitive->indices[i+2]].pos);
            triangleMesh->addTriangle(-v1, -v2, -v3, true); // Note: negative signs, this is correct for prototype tool meshes
         }

         mCollisionShape = new btBvhTriangleMeshShape(triangleMesh, true);
      }

      btRigidBody::btRigidBodyConstructionInfo constructionInfo(mass, motionState, mCollisionShape, localInertia);
      constructionInfo.m_mass = mass;
      constructionInfo.m_friction = mFriction;
      constructionInfo.m_rollingFriction = mRollingFriction;
      constructionInfo.m_restitution = mRestitution;

      mRigidBody = new btRigidBody(constructionInfo);
      mRigidBody->setUserPointer(this);
      mRigidBody->setAnisotropicFriction(ToBulletVec3(mAnisotropicFriction));
      mRigidBody->activate();

      // Initial position
      btTransform& worldTransform = mRigidBody->getWorldTransform();
      worldTransform.setOrigin(ToBulletVec3(GetTransform().GetPosition()));

      // Add to physics simulation
      gPhysics().GetDynamicsWorld()->addRigidBody(mRigidBody);

      UpdateKinematicFlag();
   }

   void CRigidBody::RemoveFromWorld()
   {
      if (mRigidBody == nullptr)
         return;

      gPhysics().GetDynamicsWorld()->removeRigidBody(mRigidBody);

      delete mRigidBody->getMotionState();
      delete mRigidBody;

      mRigidBody = nullptr;
   }

   void CRigidBody::UpdateKinematicFlag()
   {
      uint32_t flags = mRigidBody->getCollisionFlags();

      if (IsKinematic())
      {
         flags |= btCollisionObject::CF_KINEMATIC_OBJECT;

         mRigidBody->forceActivationState(DISABLE_DEACTIVATION);
      }
      else
      {
         flags &= btCollisionObject::CF_KINEMATIC_OBJECT;
      }

      mRigidBody->setCollisionFlags(flags);
   }

   bool CRigidBody::IsActive() const
   {
      return mRigidBody->isActive();
   }

   void CRigidBody::SetPosition(const glm::vec3& position)
   {
      mTransform->SetPosition(position);
   }

   void CRigidBody::SetRotation(const glm::vec3& rotation)
   {
      mTransform->SetRotation(rotation);
   }

   void CRigidBody::SetQuaternion(const glm::quat& quaternion)
   {
      mTransform->SetOrientation(quaternion);
   }

   void CRigidBody::ApplyCentralImpulse(const glm::vec3 impulse)
   {
      mRigidBody->activate(true);
      mRigidBody->applyCentralImpulse(ToBulletVec3(impulse));
   }

   void CRigidBody::ApplyCentralForce(const glm::vec3 force)
   {
      mRigidBody->activate(true);
      mRigidBody->applyCentralForce(ToBulletVec3(force));
   }

   void CRigidBody::SetVelocityXZ(const glm::vec3 velocity)
   {
      glm::vec3 currentVelocity = ToVec3(mRigidBody->getLinearVelocity());
      mRigidBody->activate(true);
      mRigidBody->setLinearVelocity(ToBulletVec3(velocity + currentVelocity.y));
   }

   void CRigidBody::SetAngularVelocity(const glm::vec3 angularVelocity)
   {
      mRigidBody->setAngularVelocity(ToBulletVec3(angularVelocity));
      mRigidBody->setAngularFactor(0.0f);
   }

   glm::vec3 CRigidBody::GetVelocity() const
   {
      return ToVec3(mRigidBody->getLinearVelocity());
   }

   const Utopian::Transform& CRigidBody::GetTransform() const
   {
      return mTransform->GetTransform();
   }

   LuaPlus::LuaObject CRigidBody::GetLuaObject()
   {
      LuaPlus::LuaObject luaObject;
      luaObject.AssignNewTable(gLuaManager().GetLuaState());

      luaObject.SetNumber("collisionShapeType", (lua_Number)mCollisionShapeType);
      luaObject.SetNumber("mass", mMass);
      luaObject.SetNumber("friction", mFriction);
      luaObject.SetNumber("rollingFriction", mRollingFriction);
      luaObject.SetNumber("restitution", mRestitution);
      luaObject.SetBoolean("kinematic", mIsKinematic);
      luaObject.SetNumber("anisotropic_friciton_x", mAnisotropicFriction.x);
      luaObject.SetNumber("anisotropic_friciton_y", mAnisotropicFriction.y);
      luaObject.SetNumber("anisotropic_friciton_z", mAnisotropicFriction.z);

      return luaObject;
   }

   float CRigidBody::GetMass() const
   {
      return mMass;
   }

   float CRigidBody::GetFriction() const
   {
      return mFriction;
   }

   float CRigidBody::GetRollingFriction() const
   {
      return mRollingFriction;
   }

   glm::vec3 CRigidBody::GetAnisotropicFriction() const
   {
      return mAnisotropicFriction;
   }

   float CRigidBody::GetRestitution() const
   {
      return mRestitution;
   }

   bool CRigidBody::IsKinematic() const
   {
      return mIsKinematic;
   }

   void CRigidBody::SetMass(float mass)
   {
      if (mMass == mass)
         return;

      mMass = mass;

      // If the body is kinematic the mass needs to be 0
      // The newly assigned mass will become active when the body becomes dynamic again
      if (IsKinematic())
         return;

      AddToWorld();
   }

   void CRigidBody::SetFriction(float friction)
   {
      mFriction = friction;
      mRigidBody->setFriction(friction);
   }

   void CRigidBody::SetRollingFriction(float rollingFriction)
   {
      mRollingFriction = rollingFriction;
      mRigidBody->setRollingFriction(rollingFriction);
   }

   void CRigidBody::SetAnisotropicFriction(const glm::vec3 anisotropicFriction)
   {
      mAnisotropicFriction = anisotropicFriction;
      mRigidBody->setAnisotropicFriction(ToBulletVec3(anisotropicFriction));
   }

   void CRigidBody::SetRestitution(float restitution)
   {
      mRestitution = restitution;
      mRigidBody->setRestitution(restitution);
   }

   void CRigidBody::SetKinematic(bool isKinematic)
   {
      if (mIsKinematic == isKinematic)
         return;

      mIsKinematic = isKinematic;

      AddToWorld();
   }

   void CRigidBody::SetCollisionShapeType(CollisionShapeType collisonShapeType)
   {
      mCollisionShapeType = collisonShapeType;
   }
}