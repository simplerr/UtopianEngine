#include "core/renderer/Renderable.h"
#include "core/renderer/SceneRenderer.h"
#include "vulkan/StaticModel.h"
#include <glm/gtc/matrix_transform.hpp>
#include "utility/math/BoundingBox.h"

namespace Utopian
{
	Renderable::Renderable()
	{
		SetPipeline(Vk::PipelineType2::BASIC);
		SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
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

	Utopian::Vk::StaticModel* Renderable::GetModel()
	{
		return mModel;
	}

	void Renderable::SetModel(Utopian::Vk::StaticModel* model)
	{
		mModel = model;
	
}

	void Renderable::SetPipeline(Vk::PipelineType2 pipeline)
	{
		mPipeline = pipeline;
	}

	void Renderable::SetColor(glm::vec4 color)
	{
		mColor = color;
	}

	const BoundingBox Renderable::GetBoundingBox() const
	{
		BoundingBox boundingBox = mModel->GetBoundingBox();
		float height = boundingBox.GetHeight();
		mat4 world;
		world = glm::translate(world, GetPosition());// +vec3(0.0f, -boundingBox.GetHeight(), 0.0f));
		world = glm::rotate(world, glm::radians(GetRotation().x), vec3(1.0f, 0.0f, 0.0f));
		world = glm::rotate(world, glm::radians(GetRotation().y), vec3(0.0f, 1.0f, 0.0f));
		world = glm::rotate(world, glm::radians(GetRotation().z), vec3(0.0f, 0.0f, 1.0f));
		world = glm::scale(world, GetScale());
		boundingBox.Update(world);

		return boundingBox;
	}

	const Vk::PipelineType2 Renderable::GetPipeline() const
	{
		return mPipeline;
	}

	const glm::vec4 Renderable::GetColor() const
	{
		return mColor;
	}
}