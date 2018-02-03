#include "scene/Renderable.h"
#include "scene/SceneRenderer.h"
#include "vulkan/StaticModel.h"

namespace Scene
{
	Renderable::Renderable()
	{

	}

	Renderable::~Renderable()
	{

	}

	SharedPtr<Renderable> Renderable::Create()
	{
		SharedPtr<Renderable> instance(new Renderable());
		instance->Initialize();

		return instance;
	}

	void Renderable::Initialize()
	{
		// Add new instance to the Renderer (scene)
		SceneRenderer::Instance().AddRenderable(this);
	}

	Vulkan::StaticModel* Renderable::GetModel()
	{
		return mModel;
	}

	void Renderable::SetModel(Vulkan::StaticModel* model)
	{
		mModel = model;
	}

	const Vulkan::BoundingBox Renderable::GetBoundingBox() const
	{
		return mModel->GetBoundingBox();
	}
}