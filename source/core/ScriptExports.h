#pragma once
#include <stdint.h>
#include "utility/PerlinNoise.h"
#include "utility/Common.h"
#include "core/LuaManager.h"

namespace Utopian
{
	class Terrain;

	class ScriptExports
	{
	public:
		static void Register();
		static void DebugPrint(const char* text);
		static void AddAsset(uint32_t assetId, float x, float y, float z, float rx, float ry, float rz, float scale);
		static void AddInstancedAsset(uint32_t assetId, float x, float y, float z, float rx, float ry, float rz, float scale, bool animated, bool castShadow);
		static void BuildInstanceBuffers();
		static void ClearInstanceGroups();
		static void SeedNoise(uint32_t seed);
		static float GetNoise(float x, float y, float z);
		static float GetTerrainHeight(float x, float z);
		//static glm::vec3 GetTerrainNormal(float x, float z);

		static void SetTerrain(const SharedPtr<Terrain>& terrain);
	private:
		static PerlinNoise<float> mPerlinNoise;
		static SharedPtr<Terrain> mTerrain;
	};

	class ScriptImports
	{
	public:
		static void Register();
		static float GetTerrainHeight(float x, float z);	
		static SharedPtr<LuaPlus::LuaFunction<float>> get_terrain_height;
	};
}
