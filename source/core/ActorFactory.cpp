#include "ActorFactory.h"
#include "LuaManager.h"
#include "core/components/Actor.h"
#include "core/components/Component.h"

namespace Utopian
{
	void ActorFactory::LoadFromFile(std::string luaFilename)
	{
	}

	void ActorFactory::SaveToFile(const std::vector<Actor*>& actors)
	{
		LuaPlus::LuaObject luaScene;
		luaScene.AssignNewTable(gLuaManager().GetLuaState());

		uint32_t counter = 0;
		for (auto& actor : actors)
		{
			LuaPlus::LuaObject luaActor;
			luaActor.AssignNewTable(gLuaManager().GetLuaState());
			luaActor.SetString("actor_name", actor->GetName().c_str());

			LuaPlus::LuaObject luaComponentTable;
			luaComponentTable.AssignNewTable(gLuaManager().GetLuaState());

			std::vector<Component*> components = actor->GetComponents();
			for (auto& component : components)
			{
				std::string name = component->GetName();
				LuaPlus::LuaObject luaObject = component->GetLuaObject();

				if (!luaObject.IsNil())
					luaComponentTable.SetObject(name.c_str(), luaObject);
			}

			luaActor.SetObject("components", luaComponentTable);
			luaScene.SetObject(counter, luaActor);
			counter++;
		}				

		gLuaManager().GetLuaState()->DumpObject("data/scene.lua", "actor_list", luaScene);
	}

	void ActorFactory::LoadActor(const LuaPlus::LuaObject& luaObject)
	{

	}
}