#include "core/renderer/Renderable.h"
#include "core/renderer/RenderingManager.h"
#include "vulkan/StaticModel.h"
#include <glm/gtc/matrix_transform.hpp>
#include "utility/math/BoundingBox.h"
#include "vulkan/ModelLoader.h"

namespace Utopian
{
	Renderable::Renderable()
	{
		SetRenderFlags(RENDER_FLAG_DEFERRED);
		SetTileFactor(glm::vec2(1.0f, 1.0f));
		SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		SetMaterial(Vk::Mat(Vk::EffectType::PHONG, Vk::PhongEffect::NORMAL));
		SetVisible(true);
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

	void Renderable::OnDestroyed()
	{
		RenderingManager::Instance().RemoveRenderable(this);
	}

	void Renderable::Initialize()
	{
		// Add new instance to the VulkanApp (scene)
		RenderingManager::Instance().AddRenderable(this);
	}

	Utopian::Vk::StaticModel* Renderable::GetModel()
	{
		return mModel;
	}

	void Renderable::LoadModel(std::string path)
	{
		mModel = Vk::gModelLoader().LoadModel(path);
	}

	void Renderable::SetModel(Utopian::Vk::StaticModel* model)
	{
		mModel = model;
	}

	void Renderable::SetTexture(Vk::Texture* texture)
	{
		mModel->mMeshes[0]->SetTexture(texture);
	}

	void Renderable::SetTileFactor(glm::vec2 tileFactor)
	{
		mTextureTileFactor = tileFactor;
	}

	void Renderable::SetColor(glm::vec4 color)
	{
		mColor = color;
	}

	void Renderable::SetMaterial(Utopian::Vk::Mat material)
	{
		mMaterial = material;
	}

	void Renderable::SetVisible(bool visible)
	{
		mVisible = visible;
	}

	void Renderable::SetRenderFlags(uint32_t renderFlags)
	{
		mRenderFlags = renderFlags;
	}

	void Renderable::AppendRenderFlags(uint32_t renderFlags)
	{
		mRenderFlags |= renderFlags;
	}

	const BoundingBox Renderable::GetBoundingBox() const
	{
		BoundingBox boundingBox = mModel->GetBoundingBox();
		float height = boundingBox.GetHeight();
		glm::mat4 world;
		world = glm::translate(world, GetPosition());// +vec3(0.0f, -boundingBox.GetHeight(), 0.0f));
		world = glm::rotate(world, glm::radians(GetRotation().x), glm::vec3(1.0f, 0.0f, 0.0f));
		world = glm::rotate(world, glm::radians(GetRotation().y), glm::vec3(0.0f, 1.0f, 0.0f));
		world = glm::rotate(world, glm::radians(GetRotation().z), glm::vec3(0.0f, 0.0f, 1.0f));
		world = glm::scale(world, GetScale());
		boundingBox.Update(world);

		return boundingBox;
	}

	const glm::vec4 Renderable::GetColor() const
	{
		return mColor;
	}

	const Utopian::Vk::Mat Renderable::GetMaterial() const
	{
		return mMaterial;
	}

	glm::vec2 Renderable::GetTextureTiling() const
	{
		return mTextureTileFactor;
	}

	const bool Renderable::IsVisible() const
	{
		return mVisible;
	}

	const uint32_t Renderable::GetRenderFlags() const
	{
		return mRenderFlags;
	}

	const bool Renderable::HasRenderFlags(uint32_t renderFlags) const
	{
		return (mRenderFlags & renderFlags) == renderFlags;
	}
}