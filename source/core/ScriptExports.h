#pragma once
#include <stdint.h>
#include "utility/PerlinNoise.h"
#include "utility/Common.h"
#include "core/LuaManager.h"

namespace Utopian
{
	class ScriptExports
	{
	public:
		static void Register();
		static void DebugPrint(const char* text);
		static void AddAsset(uint32_t assetId, float x, float y, float z, float rx, float ry, float rz, float scale, bool instanced);
		static void AddInstancedAsset(uint32_t assetId, float x, float y, float z, float rx, float ry, float rz, float scale);
		static void BuildInstanceBuffers();
		static void ClearInstanceGroups();
		static void SeedNoise(uint32_t seed);
		static float GetNoise(float x, float y, float z);
		// GetTerrainHeight(float x, float z);
		// GetTerrainNormal(float x, float z);
	private:
		static PerlinNoise<float> mPerlinNoise;
	};

	class ScriptImports
	{
	public:
		static void Register();
		static float GetTerrainHeight(float x, float z);	
		static SharedPtr<LuaPlus::LuaFunction<float>> get_terrain_height;
	};
}
