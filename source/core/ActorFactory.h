#pragma once
#include <string>
#include <vector>
#include "LuaPlus.h"

namespace Utopian
{
	class Actor;
	class Window;

	class ActorFactory
	{
	public:
		//ActorFactory();
		//~ActorFactory();

		static void LoadFromFile(Window* window, std::string filename); // Note: Window should not be here
		static void SaveToFile(std::string filename, const std::vector<Actor*>& actors);
	private:
		void LoadActor(const LuaPlus::LuaObject& luaObject);
	};
}
