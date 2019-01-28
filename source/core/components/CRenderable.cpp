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

		//mInternal = Renderer::Instance().CreateRenderable();
		World::Instance().BindNode(mInternal, GetParent());
	}

	void CRenderable::OnDestroyed()
	{
		mInternal->OnDestroyed();
		World::Instance().RemoveNode(mInternal);
	}

	void CRenderable::PostInit()
	{
	}

	LuaPlus::LuaObject CRenderable::GetLuaObject()
	{
		LuaPlus::LuaObject luaObject;
		luaObject.AssignNewTable(gLuaManager().GetLuaState());

		luaObject.SetString("path", GetPath().c_str());
		luaObject.SetNumber("render_flags", GetRenderFlags());

		return luaObject;
	}

	void CRenderable::LoadModel(std::string path)
	{
		mPath = path;
		mInternal->LoadModel(path);
	}

	void CRenderable::SetModel(Utopian::Vk::StaticModel* model)
	{
		// Note: Todo: How should ActorFactory that loads from Lua handle this?
		mPath = "Unknown";
		mInternal->SetModel(model);
	}

	void CRenderable::SetTexture(Vk::Texture* texture)
	{
		mInternal->SetTexture(texture);
	}

	void CRenderable::SetTileFactor(glm::vec2 tileFactor)
	{
		mInternal->SetTileFactor(tileFactor);
	}

	void CRenderable::SetColor(glm::vec4 color)
	{
		mInternal->SetColor(color);
	}

	void CRenderable::SetMaterial(Utopian::Vk::Mat material)
	{
		mInternal->SetMaterial(material);
	}

	void CRenderable::SetRenderFlags(uint32_t renderFlags)
	{
		mInternal->SetRenderFlags(renderFlags);
	}

	void CRenderable::AppendRenderFlags(uint32_t renderFlags)
	{
		mInternal->SetRenderFlags(mInternal->GetRenderFlags() | renderFlags);
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
	
	uint32_t CRenderable::GetRenderFlags() const
	{
		return mInternal->GetRenderFlags();
	}

	glm::vec2 CRenderable::GetTextureTiling() const
	{
		return mInternal->GetTextureTiling();
	}

	const bool CRenderable::HasRenderFlags(uint32_t renderFlags) const
	{
		return mInternal->HasRenderFlags(renderFlags);
	}
}