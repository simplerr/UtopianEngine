#include "core/ScriptExports.h"
#include "core/LuaManager.h"
#include "core/AssetLoader.h"
#include "core/components/Actor.h"
#include "core/components/CRenderable.h"
#include "core/components/CTransform.h"
#include "vulkan/VulkanDebug.h"
#include "utility/Common.h"

namespace Utopian
{
	void ScriptExports::Register()
	{
		LuaPlus::LuaObject globals = gLuaManager().GetGlobalVars();

		globals.RegisterDirect("debug_print", &ScriptExports::DebugPrint);
		globals.RegisterDirect("add_asset", &ScriptExports::AddAsset);
	}

	void ScriptExports::DebugPrint(const char* text)
	{
		Vk::VulkanDebug::ConsolePrint(text);
	}

	void ScriptExports::AddAsset(uint32_t assetId, float x, float y, float z, float scale)
	{
		SharedPtr<Actor> actor = Actor::Create("Lua asset");

		CTransform* transform = actor->AddComponent<CTransform>(glm::vec3(x, y, z));
		transform->AddRotation(glm::vec3(180.0f, 0, 0));
		transform->SetScale(glm::vec3(scale));

		CRenderable* renderable = actor->AddComponent<CRenderable>();
		renderable->SetModel(gAssetLoader().LoadAsset(assetId));
	}
}