#include "core/components/CRenderable.h"
#include "core/components/Actor.h"
#include "core/renderer/Renderable.h"
#include "core/components/CTransform.h"
#include "core/World.h"

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

	void CRenderable::SetColor(glm::vec4 color)
	{
		mInternal->SetColor(color);
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