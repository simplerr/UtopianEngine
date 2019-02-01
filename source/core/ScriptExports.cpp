#include "core/ScriptExports.h"
#include "core/AssetLoader.h"
#include "core/components/Actor.h"
#include "core/components/CRenderable.h"
#include "core/components/CTransform.h"
#include "core/renderer/Renderer.h"
#include "vulkan/Debug.h"
#include "utility/Common.h"

namespace Utopian
{
	PerlinNoise<float> ScriptExports::mPerlinNoise;
	SharedPtr<LuaPlus::LuaFunction<float>> ScriptImports::get_terrain_height;

	void ScriptExports::Register()
	{
		LuaPlus::LuaObject globals = gLuaManager().GetGlobalVars();

		globals.RegisterDirect("debug_print", &ScriptExports::DebugPrint);
		globals.RegisterDirect("add_asset", &ScriptExports::AddAsset);
		globals.RegisterDirect("add_instanced_asset", &ScriptExports::AddInstancedAsset);
		globals.RegisterDirect("build_instance_buffers", &ScriptExports::BuildInstanceBuffers);
		globals.RegisterDirect("clear_instance_groups", &ScriptExports::ClearInstanceGroups);
		globals.RegisterDirect("seed_noise", &ScriptExports::SeedNoise);
		globals.RegisterDirect("get_noise", &ScriptExports::GetNoise);
	}

	void ScriptExports::DebugPrint(const char* text)
	{
		Vk::Debug::ConsolePrint(text);
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
			gRenderer().AddInstancedAsset(assetId, glm::vec3(x, y, z), glm::vec3(rx, ry, rz), glm::vec3(scale));
		}
	}

	void ScriptExports::AddInstancedAsset(uint32_t assetId, float x, float y, float z, float rx, float ry, float rz, float scale)
	{
		gRenderer().AddInstancedAsset(assetId, glm::vec3(x, y, z), glm::vec3(rx, ry, rz), glm::vec3(scale));
	}

	void ScriptExports::BuildInstanceBuffers()
	{
		gRenderer().BuildAllInstances();
	}

	void ScriptExports::ClearInstanceGroups()
	{
		gRenderer().ClearInstanceGroups();
	}

	void ScriptExports::SeedNoise(uint32_t seed)
	{
		mPerlinNoise.Seed(seed);
	}

	float ScriptExports::GetNoise(float x, float y, float z)
	{
		return mPerlinNoise.Noise(x, y, z);
	}

	void ScriptImports::Register()
	{
		LuaPlus::LuaObject object = gLuaManager().GetGlobalVars()["get_terrain_height"];
		if (object.IsFunction())
		{
			get_terrain_height = std::make_shared<LuaPlus::LuaFunction<float>>(object);
		}
		else
		{
			Vk::Debug::ConsolePrint("get_terrain_height() functin not found in Lua");
		}
	}

	float ScriptImports::GetTerrainHeight(float x, float z)
	{
		// Note: Function from Lua is retrieved in Register() to speed it up 
		float height = get_terrain_height.get()->operator()(x, z);
		return height;
	}	
}