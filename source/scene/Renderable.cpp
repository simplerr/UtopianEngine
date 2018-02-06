#include "scene/Renderable.h"
#include "scene/SceneRenderer.h"
#include "vulkan/StaticModel.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Collision.h"

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
		Vulkan::BoundingBox boundingBox = mModel->GetBoundingBox();
		mat4 world;
		world = glm::translate(world, GetPosition() + vec3(0.0f, boundingBox.GetHeight() / 2, 0.0f));
		/*world = glm::rotate(world, glm::radians(GetRotation().x), vec3(1.0f, 0.0f, 0.0f));
		world = glm::rotate(world, glm::radians(GetRotation().y), vec3(0.0f, 1.0f, 0.0f));
		world = glm::rotate(world, glm::radians(GetRotation().z), vec3(0.0f, 0.0f, 1.0f));*/
		world = glm::scale(world, GetScale());
		boundingBox.Update(world);

		return boundingBox;
	}
}