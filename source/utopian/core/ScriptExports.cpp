#include "core/ScriptExports.h"
#include "core/AssetLoader.h"
#include "core/components/Actor.h"
#include "core/components/CRenderable.h"
#include "core/components/CTransform.h"
#include "core/renderer/Renderer.h"
#include "core/Log.h"
#include "utility/Common.h"

namespace Utopian
{
   PerlinNoise<float> ScriptExports::mPerlinNoise;
   Terrain* ScriptExports::mTerrain;
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
      globals.RegisterDirect("get_terrain_height", &ScriptExports::GetTerrainHeight);
      //globals.RegisterDirect("get_terrain_normal", &ScriptExports::GetTerrainNormal);
   }

   void ScriptExports::DebugPrint(const char* text)
   {
      UTO_LOG(text);
   }

   void ScriptExports::AddAsset(uint32_t assetId, float x, float y, float z, float rx, float ry, float rz, float scale)
   {
      SharedPtr<Actor> actor = Actor::Create("Lua asset");

      CTransform* transform = actor->AddComponent<CTransform>(glm::vec3(x, y, z));
      transform->AddRotation(glm::vec3(rx, ry, rz));
      transform->SetScale(glm::vec3(scale));

      CRenderable* renderable = actor->AddComponent<CRenderable>();
      renderable->SetModel(gAssetLoader().LoadAsset(assetId));
   }

   void ScriptExports::AddInstancedAsset(uint32_t assetId, float x, float y, float z, float rx, float ry, float rz, float scale, bool animated, bool castShadow)
   {
      gRenderer().AddInstancedAsset(assetId, glm::vec3(x, y, z), glm::vec3(rx, ry, rz), glm::vec3(scale), animated, castShadow);
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
   
   float ScriptExports::GetTerrainHeight(float x, float z)
   {
      return mTerrain->GetHeight(x, z);
   }

   //glm::vec3 ScriptExports::GetTerrainNormal(float x, float z)
   //{
   // return mTerrain->GetNormal(x, z);
   //}

   void ScriptExports::SetTerrain(Terrain* terrain)
   {
      mTerrain = terrain;
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
         UTO_LOG("get_terrain_height() functin not found in Lua");
      }
   }

   float ScriptImports::GetTerrainHeight(float x, float z)
   {
      // Note: Function from Lua is retrieved in Register() to speed it up 
      float height = get_terrain_height.get()->operator()(x, z);
      return height;
   }
}