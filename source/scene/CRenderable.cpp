#include "scene/CRenderable.h"
#include "scene/Actor.h"
#include "scene/Renderable.h"
#include "scene/CTransform.h"
#include "scene/World.h"

namespace Scene
{
	CRenderable::CRenderable(Actor* parent)
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

	void CRenderable::EnableBoundingBox()
	{
		mInternal->SetDrawBoundingBox(true);
	}

	void CRenderable::DisableBoundingBox()
	{
		mInternal->SetDrawBoundingBox(false);
	}

	const Vulkan::BoundingBox CRenderable::GetBoundingBox() const
	{
		return mInternal->GetBoundingBox();
	}
}