#include "LuaManager.h"
#include <iostream>

namespace Utopian
{
	LuaManager::LuaManager()
	{
		Init();
	}

	LuaManager::~LuaManager()
	{
	}

	LuaManager& gLuaManager()
	{
		return LuaManager::Instance();
	}

	bool LuaManager::Init()
	{
		mLuaState = LuaPlus::LuaState::Create(true);
		if (mLuaState == NULL)
			return false;

		// register functions
		mLuaState->GetGlobals().RegisterDirect("ExecuteFile", (*this), &LuaManager::ExecuteFile);
		mLuaState->GetGlobals().RegisterDirect("ExecuteString", (*this), &LuaManager::ExecuteString);
		mLuaState->GetGlobals().RegisterDirect("PrintTable", (*this), &LuaManager::PrintTable);

		return true;
	}

	void LuaManager::ExecuteFile(const char* path)
	{
		int result = mLuaState->DoFile(path);
		if (result != 0)
			SetError(result);
	}

	void LuaManager::ExecuteString(const char* str)
	{

	}

	void LuaManager::SetError(int errorNum)
	{
		// Note: If we get an error, we're hosed because LuaPlus throws an exception.  So if this function
		// is called and the error at the bottom triggers, you might as well pack it in.

		LuaPlus::LuaStackObject stackObj(mLuaState, -1);
		const char* errStr = stackObj.GetString();
		if (errStr)
		{
			mLastError = errStr;
			mLuaState->SetTop(0);
		}
		else
			mLastError = "Unknown Lua parse error";

		//GLIB_ERROR(mLastError);
	}

	LuaPlus::LuaObject LuaManager::GetGlobalVars()
	{
		return mLuaState->GetGlobals();
	}

	LuaPlus::LuaObject LuaManager::GetGlobal(std::string name)
	{
		return mLuaState->GetGlobal(name.c_str());
	}

	LuaPlus::LuaState* LuaManager::GetLuaState() const
	{
		return mLuaState;
	}

	void LuaManager::PrintTable(LuaPlus::LuaObject table, bool recursive)
	{
		if (table.IsNil())
		{
			//if (!recursive)
			//	GLIB_LOG("LuaManager", "null table");
			return;
		}

		//if (!recursive)
		//	GLIB_LOG("LuaManager", "PrintTable()");

		std::cout << (!recursive ? "--\n" : "{ ");

		bool noMembers = true;
		for (LuaPlus::LuaTableIterator iter(table); iter; iter.Next())
		{
			noMembers = false;

			LuaPlus::LuaObject key = iter.GetKey();
			LuaPlus::LuaObject value = iter.GetValue();

			std::cout << key.ToString() << ": ";
			if (value.IsFunction())
				std::cout << "function" << "\n";
			else if (value.IsTable())
				PrintTable(value, true);
			else if (value.IsLightUserdata())
				std::cout << "light user data" << "\n";
			else
				std::cout << value.ToString() << ", ";
		}

		std::cout << (!recursive ? "\n--" : "}");
		std::cout << std::endl;
	}
}

//void LuaManager::ConvertVec3ToTable(const XMFLOAT3& vec, LuaPlus::LuaObject& outLuaTable) const
//{
//	outLuaTable.AssignNewTable(GetLuaState());
//	outLuaTable.SetNumber("x", vec.x);
//	outLuaTable.SetNumber("y", vec.y);
//	outLuaTable.SetNumber("z", vec.z);
//}