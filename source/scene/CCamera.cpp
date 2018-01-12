#include "scene/CCamera.h"
#include "scene/World.h"

namespace Scene
{
	CCamera::CCamera(SceneEntity* parent)
		: SceneComponent(parent)
	{

	}

	CCamera::~CCamera()
	{

	}

	void CCamera::Update()
	{

	}

	void CCamera::OnCreated()
	{
		//mInternal = Camera::Create();

		//World::Instance().BindNode(mInternal, GetParent());
	}
}