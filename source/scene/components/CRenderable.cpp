#include "scene/components/CRenderable.h"
#include "scene/components/Actor.h"
#include "scene/Renderable.h"
#include "scene/components/CTransform.h"
#include "scene/World.h"

namespace Utopian
{
	CRenderable::CRenderable(Actor* parent)
		: Component(parent)
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

	void CRenderable::SetModel(Utopian::Vk::StaticModel* model)
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

	const BoundingBox CRenderable::GetBoundingBox() const
	{
		return mInternal->GetBoundingBox();
	}
}