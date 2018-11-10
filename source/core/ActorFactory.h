#pragma once
#include <string>
#include <vector>
#include "LuaPlus.h"

namespace Utopian
{
	class Actor;

	class ActorFactory
	{
	public:
		//ActorFactory();
		//~ActorFactory();

		static void LoadFromFile(std::string luaFilename);
		static void SaveToFile(const std::vector<Actor*>& actors);
	private:
		void LoadActor(const LuaPlus::LuaObject& luaObject);
	};
}
