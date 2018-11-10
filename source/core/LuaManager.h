#pragma once

#include "LuaPlus.h"
#include "utility/Module.h"
#include <string>

namespace Utopian
{
	class LuaManager : public Module<LuaManager>
	{
	public:
		LuaManager();
		virtual ~LuaManager();

		static bool Create();
		static void Destroy();
		static LuaManager* Get();

		bool Init();
		void ExecuteFile(const char* path);
		void ExecuteString(const char* str);

		void SetError(int errorNum);

		LuaPlus::LuaObject GetGlobalVars();
		LuaPlus::LuaObject GetGlobal(std::string name);
		LuaPlus::LuaState* GetLuaState() const;

		// Debug functions
		void PrintTable(LuaPlus::LuaObject table, bool recursive = false);

		//void ConvertVec3ToTable(const XMFLOAT3& vec, LuaPlus::LuaObject& outLuaTable) const;
	private:
		LuaPlus::LuaState* mLuaState;
		std::string mLastError;
	};

	LuaManager& gLuaManager();
}
