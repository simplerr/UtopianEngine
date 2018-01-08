#include "scene/CRenderable.h"
#include "scene/SceneEntity.h"
#include "scene/Renderable.h"
#include "scene/CTransform.h"
#include "scene/World.h"

namespace Scene
{
	CRenderable::CRenderable(SceneEntity* parent)
		: SceneComponent(parent)
	{
		SetName("CStaticMesh");
	}

	CRenderable::~CRenderable()
	{

	}

	void CRenderable::Update()
	{

	}

	void CRenderable::OnCreated()
	{
		mInternal = Renderable::Create();

		World::Instance().BindNode(mInternal, GetParent());
	}

	void CRenderable::SetModel(Vulkan::StaticModel* model)
	{
		mInternal->SetModel(model);
	}
}