#include "core/ScriptExports.h"
#include "core/LuaManager.h"
#include "core/AssetLoader.h"
#include "core/components/Actor.h"
#include "core/components/CRenderable.h"
#include "core/components/CTransform.h"
#include "core/renderer/RenderingManager.h"
#include "vulkan/VulkanDebug.h"
#include "utility/Common.h"

namespace Utopian
{
	void ScriptExports::Register()
	{
		LuaPlus::LuaObject globals = gLuaManager().GetGlobalVars();

		globals.RegisterDirect("debug_print", &ScriptExports::DebugPrint);
		globals.RegisterDirect("add_asset", &ScriptExports::AddAsset);
		globals.RegisterDirect("add_instanced_asset", &ScriptExports::AddInstancedAsset);
		globals.RegisterDirect("build_instance_buffers", &ScriptExports::BuildInstanceBuffers);
		globals.RegisterDirect("clear_instance_groups", &ScriptExports::ClearInstanceGroups);
	}

	void ScriptExports::DebugPrint(const char* text)
	{
		Vk::VulkanDebug::ConsolePrint(text);
	}

	void ScriptExports::AddAsset(uint32_t assetId, float x, float y, float z, float rx, float ry, float rz, float scale, bool instanced)
	{
		if (!instanced)
		{
			SharedPtr<Actor> actor = Actor::Create("Lua asset");

			CTransform* transform = actor->AddComponent<CTransform>(glm::vec3(x, y, z));
			transform->AddRotation(glm::vec3(rx, ry, rz));
			transform->SetScale(glm::vec3(scale));

			CRenderable* renderable = actor->AddComponent<CRenderable>();
			renderable->SetModel(gAssetLoader().LoadAsset(assetId));
		}
		else
		{
			gRenderingManager().AddInstancedAsset(assetId, glm::vec3(x, y, z), glm::vec3(rx, ry, rz), glm::vec3(scale));
		}
	}

	void ScriptExports::AddInstancedAsset(uint32_t assetId, float x, float y, float z, float rx, float ry, float rz, float scale)
	{
		gRenderingManager().AddInstancedAsset(assetId, glm::vec3(x, y, z), glm::vec3(rx, ry, rz), glm::vec3(scale));
	}

	void ScriptExports::BuildInstanceBuffers()
	{
		gRenderingManager().BuildAllInstances();
	}

	void ScriptExports::ClearInstanceGroups()
	{
		gRenderingManager().ClearInstanceGroups();
	}
}