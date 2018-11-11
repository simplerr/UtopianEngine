#include "core/components/CRenderable.h"
#include "core/components/Actor.h"
#include "core/renderer/Renderable.h"
#include "core/components/CTransform.h"
#include "core/World.h"
#include "vulkan/ModelLoader.h"

namespace Utopian
{
	CRenderable::CRenderable(Actor* parent)
		: Component(parent)
	{
		SetName("CRenderable");
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

	void CRenderable::PostInit()
	{
	}

	LuaPlus::LuaObject CRenderable::GetLuaObject()
	{
		LuaPlus::LuaObject luaObject;
		luaObject.AssignNewTable(gLuaManager().GetLuaState());

		luaObject.SetString("path", GetPath().c_str());

		return luaObject;
	}

	void CRenderable::LoadModel(std::string path)
	{
		mPath = path;
		mInternal->LoadModel(path);
	}

	void CRenderable::SetModel(std::string path, Utopian::Vk::StaticModel* model)
	{
		mPath = path;
		mInternal->SetModel(model);
	}

	void CRenderable::SetColor(glm::vec4 color)
	{
		mInternal->SetColor(color);
	}

	void CRenderable::SetMaterial(Utopian::Vk::Mat material)
	{
		mInternal->SetMaterial(material);
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

	const std::string CRenderable::GetPath() const
	{
		return mPath;
	}
}