#include "scene/CRenderable.h"
#include "scene/SceneEntity.h"
#include "scene/Renderable.h"
#include "scene/CTransform.h"

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
		// TODO: Move to SceneManager
		auto transform = GetParent()->GetComponent<CTransform>();
		mInternal->pos = transform->GetPosition();
		mInternal->scale = transform->GetScale();
	}
	void CRenderable::OnCreated()
	{
		mInternal = Renderable::Create();

		// TODO:
		//SceneManager::BindActor(mInternal, GetParent());
	}

	void CRenderable::SetModel(Vulkan::StaticModel* model)
	{
		mInternal->SetModel(model);
	}

	// onInitialized()
	// SceneManager::BindActor(mInternal, GetParent())
}