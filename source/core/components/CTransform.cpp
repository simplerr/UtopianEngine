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

	CTransform::CTransform(Actor* parent)
		: Component(parent)
	{
		SetName("CTransform");
		SetPosition(glm::vec3(0, 0, 0));
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

		luaObject.SetNumber("scale_x", GetScale().x);
		luaObject.SetNumber("scale_y", GetScale().y);
		luaObject.SetNumber("scale_z", GetScale().z);

		glm::quat orientation = GetOrientation();
		luaObject.SetNumber("orientation_x", orientation.x);
		luaObject.SetNumber("orientation_y", orientation.y);
		luaObject.SetNumber("orientation_z", orientation.z);
		luaObject.SetNumber("orientation_w", orientation.w);

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

	void CTransform::SetRotation(const glm::vec3& eulerRotation)
	{
		mTransform.SetRotation(eulerRotation);
	}

	void CTransform::SetScale(const glm::vec3& scale)
	{
		mTransform.SetScale(scale);
	}

	void CTransform::SetOrientation(const glm::quat& orientation)
	{
		mTransform.SetOrientation(orientation);
	}

	void CTransform::AddTranslation(const glm::vec3& translation)
	{
		mTransform.AddTranslation(translation);
	}

	void CTransform::AddRotation(const glm::vec3& eulerRotation, bool local)
	{
		mTransform.AddRotation(eulerRotation, local);
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

	const glm::quat& CTransform::GetOrientation() const
	{
		return mTransform.GetOrientation();
	}
}