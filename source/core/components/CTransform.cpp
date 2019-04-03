#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "core/components/CTransform.h"

namespace Utopian
{
	CTransform::CTransform(Actor* parent, const glm::vec3& position)
		: Component(parent)
	{
		SetName("CTransform");
		SetPosition(position);
		SetRotation(glm::vec3(0, 0, 0));
		SetScale(glm::vec3(1.0f, 1.0f, 1.0f));
	}

	CTransform::~CTransform()
	{
	}

	void CTransform::PostInit()
	{
	}

	LuaPlus::LuaObject CTransform::GetLuaObject()
	{
		LuaPlus::LuaObject luaObject;
		luaObject.AssignNewTable(gLuaManager().GetLuaState());

		luaObject.SetNumber("pos_x", GetPosition().x);
		luaObject.SetNumber("pos_y", GetPosition().y);
		luaObject.SetNumber("pos_z", GetPosition().z);

		luaObject.SetNumber("rotation_x", GetRotation().x);
		luaObject.SetNumber("rotation_y", GetRotation().y);
		luaObject.SetNumber("rotation_z", GetRotation().z);

		luaObject.SetNumber("scale_x", GetScale().x);
		luaObject.SetNumber("scale_y", GetScale().y);
		luaObject.SetNumber("scale_z", GetScale().z);

		return luaObject;
	}

	void CTransform::SetTransform(const Transform& transform)
	{
		mTransform = transform;
	}

	void CTransform::SetPosition(const glm::vec3& position)
	{
		mTransform.SetPosition(position);
	}

	void CTransform::SetRotation(const glm::vec3& rotation)
	{
		mTransform.SetRotation(rotation);
	}

	void CTransform::SetScale(const glm::vec3& scale)
	{
		mTransform.SetScale(scale);
	}

	void CTransform::SetQuaternion(const glm::quat& quaternion)
	{
		mTransform.SetQuaternion(quaternion);
	}

	void CTransform::AddTranslation(const glm::vec3& translation)
	{
		mTransform.AddTranslation(translation);
	}

	void CTransform::AddRotation(float x, float y, float z)
	{
		mTransform.AddRotation(x, y, z);
	}
	
	void CTransform::AddRotation(const glm::vec3& rotation)
	{
		mTransform.AddRotation(rotation);
	}

	void CTransform::AddScale(float x, float y, float z)
	{
		mTransform.AddScale(x, y, z);
	}

	void CTransform::AddScale(const glm::vec3& scale)
	{
		mTransform.AddScale(scale);
	}

	const Utopian::Transform& CTransform::GetTransform() const
	{
		return mTransform;
	}

	const glm::vec3& CTransform::GetPosition() const
	{
		return mTransform.GetPosition();
	}

	const glm::vec3& CTransform::GetRotation() const
	{
		return mTransform.GetRotation();
	}

	const glm::vec3& CTransform::GetScale() const
	{
		return mTransform.GetScale();
	}

	const glm::mat4& CTransform::GetWorldMatrix() const
	{
		return mTransform.GetWorldMatrix();
	}

	const glm::mat4& CTransform::GetWorldInverseTransposeMatrix() const
	{
		return mTransform.GetWorldInverseTransposeMatrix();
	}

	const glm::quat& CTransform::GetQuaternion() const
	{
		return mTransform.GetQuaternion();
	}
}