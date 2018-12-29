#pragma once
#include <stdint.h>

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
		// GetTerrainHeight(float x, float z);
		// GetTerrainNormal(float x, float z);
	};
}
