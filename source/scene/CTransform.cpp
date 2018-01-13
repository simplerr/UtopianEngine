#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "scene/CTransform.h"

namespace Scene
{
	CTransform::CTransform(SceneEntity* parent, const vec3& position)
		: SceneComponent(parent)
	{
		SetName("CTransform");
		SetPosition(position);
		SetRotation(vec3(0, 0, 0));
		SetScale(vec3(1.0f, 1.0f, 1.0f));
	}

	CTransform::~CTransform()
	{
	}


	void CTransform::SetTransform(const Transform& transform)
	{
		mTransform = transform;
	}

	void CTransform::SetPosition(const vec3& position)
	{
		mTransform.SetPosition(position);
	}

	void CTransform::SetRotation(const vec3& rotation)
	{
		mTransform.SetRotation(rotation);
	}

	void CTransform::SetScale(const vec3& scale)
	{
		mTransform.SetScale(scale);
	}

	void CTransform::AddTranslation(const vec3& translation)
	{
		mTransform.AddTranslation(translation);
	}

	void CTransform::AddRotation(float x, float y, float z)
	{
		mTransform.AddRotation(x, y, z);
	}
	
	void CTransform::AddRotation(const vec3& rotation)
	{
		mTransform.AddRotation(rotation);
	}

	void CTransform::AddScale(float x, float y, float z)
	{
		mTransform.AddScale(x, y, z);
	}

	void CTransform::AddScale(const vec3& scale)
	{
		mTransform.AddScale(scale);
	}


	const Scene::Transform& CTransform::GetTransform() const
	{
		return mTransform;
	}

	const vec3& CTransform::GetPosition() const
	{
		return mTransform.GetPosition();
	}

	const vec3& CTransform::GetRotation() const
	{
		return mTransform.GetRotation();
	}

	const vec3& CTransform::GetScale() const
	{
		return mTransform.GetScale();
	}

	const mat4& CTransform::GetWorldMatrix() const
	{
		return mTransform.GetWorldMatrix();
	}

	const mat4& CTransform::GetWorldInverseTransposeMatrix() const
	{
		return mTransform.GetWorldInverseTransposeMatrix();
	}
}