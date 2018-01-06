#include "scene/Renderable.h"
#include "scene/ActorRenderer.h"

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
		ActorRenderer::Instance().AddRenderable(this);
	}

	Vulkan::StaticModel* Renderable::GetModel()
	{
		return mModel;
	}

	void Renderable::SetModel(Vulkan::StaticModel* model)
	{
		mModel = model;
	}
}