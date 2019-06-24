#include "ActorFactory.h"
#include "LuaManager.h"
#include "core/components/Actor.h"
#include "core/components/Component.h"
#include "core/components/CTransform.h"
#include "core/components/CLight.h"
#include "core/components/CCamera.h"
#include "core/components/CPlayerControl.h"
#include "core/components/CNoClip.h"
#include "core/components/CRenderable.h"
#include "core/components/CBloomLight.h"
#include "core/components/CCatmullSpline.h"
#include "vulkan/Debug.h"

namespace Utopian
{
	void ActorFactory::LoadFromFile(Window* window, std::string filename)
	{
		Vk::Debug::ConsolePrint("Loading actors from file...");

		gLuaManager().ExecuteFile(filename.c_str());

		LuaPlus::LuaObject luaGlobals = gLuaManager().GetLuaState()->GetGlobal("actor_list");

		if (luaGlobals.IsNil())
			assert(0);

		// Iterate over all actors
		for (LuaPlus::LuaTableIterator luaActor(luaGlobals); luaActor; luaActor.Next())
		{
			LuaPlus::LuaObject key = luaActor.GetKey();
			LuaPlus::LuaObject actorData = luaActor.GetValue();

			std::string name = actorData["actor_name"].ToString();

			SharedPtr<Actor> actor = Actor::Create(name);

			LuaPlus::LuaObject components = actorData["components"];
			for (LuaPlus::LuaTableIterator luaComponent(components); luaComponent; luaComponent.Next())
			{
				LuaPlus::LuaObject keyComponent = luaComponent.GetKey();
				LuaPlus::LuaObject componentData = luaComponent.GetValue();

				std::string name = keyComponent.ToString();

				if (name == "CTransform") {
					glm::vec3 position(componentData["pos_x"].ToNumber(), componentData["pos_y"].ToNumber(), componentData["pos_z"].ToNumber());
					glm::vec3 scale(componentData["scale_x"].ToNumber(), componentData["scale_y"].ToNumber(), componentData["scale_z"].ToNumber());
					glm::quat orientation = glm::quat(componentData["orientation_w"].ToNumber(),
													  componentData["orientation_x"].ToNumber(),
													  componentData["orientation_y"].ToNumber(),
													  componentData["orientation_z"].ToNumber());

					CTransform* transform = actor->AddComponent<CTransform>(position);
					transform->SetOrientation(orientation);
					transform->SetScale(scale);
				}
				else if (name == "CLight") {
					glm::vec3 color(componentData["color_r"].ToNumber(), componentData["color_g"].ToNumber(), componentData["color_b"].ToNumber());
					glm::vec3 att(componentData["att_x"].ToNumber(), componentData["att_y"].ToNumber(), componentData["att_z"].ToNumber());
					glm::vec3 dir(componentData["dir_x"].ToNumber(), componentData["dir_y"].ToNumber(), componentData["dir_z"].ToNumber());
					glm::vec3 intensity(componentData["intensity_x"].ToNumber(), componentData["intensity_y"].ToNumber(), componentData["intensity_z"].ToNumber());
					uint32_t type = componentData["type"].ToInteger();
					float spot = componentData["spot"].ToNumber();
					float range = componentData["range"].ToNumber();

					CLight* light = actor->AddComponent<CLight>();
					light->SetMaterial(glm::vec4(color, 1.0f));
					light->SetDirection(dir);
					light->SetAtt(att.x, att.y, att.z);
					light->SetIntensity(intensity);
					light->SetType((LightType)type);
					light->SetRange(range);
					light->SetSpot(spot);
				}
				else if (name == "CCamera") {
					glm::vec3 target(componentData["look_at_x"].ToNumber(), componentData["look_at_y"].ToNumber(), componentData["look_at_z"].ToNumber());
					float fov = componentData["fov"].ToNumber();
					float nearPlane = componentData["near_plane"].ToNumber();
					float farPlane = componentData["far_plane"].ToNumber();
					CCamera* camera = actor->AddComponent<CCamera>(window, fov, nearPlane, farPlane);
					camera->LookAt(target);
					camera->SetMainCamera(); // Note: Currently this only suppo
				}
				else if (name == "CNoClip") {
					float speed = componentData["speed"].ToNumber();
					CNoClip* noclip = actor->AddComponent<CNoClip>(speed);

					// Testing
					Utopian::CCatmullSpline* spline = actor->AddComponent<Utopian::CCatmullSpline>();

					float y = -1300.0f;
					spline->AddControlPoint(glm::vec3(0.0f, y, 0.0f));
					spline->AddControlPoint(glm::vec3(0.0f, y + 200, 500.0f));
					spline->AddControlPoint(glm::vec3(500.0f, y, 200.0f));
					spline->AddControlPoint(glm::vec3(500.0f, y + 100, 0.0f));

					spline->AddControlPoint(glm::vec3(1000.0f, y, 0.0f));
					spline->AddControlPoint(glm::vec3(500.0f, y + 200, 500.0f));
					spline->AddControlPoint(glm::vec3(0.0f, y + 200, 500.0f));
					spline->AddControlPoint(glm::vec3(500.0f, y + 100, -300.0f));
				}
				else if (name == "CPlayerControl") {
					CPlayerControl* playerControl = actor->AddComponent<CPlayerControl>();
				}
				else if (name == "CRenderable") {
					std::string path = componentData["path"].ToString();
					uint32_t renderFlags = componentData["render_flags"].ToNumber();
					glm::vec4 color(componentData["color_r"].ToNumber(), componentData["color_g"].ToNumber(), componentData["color_b"].ToNumber(), componentData["color_a"].ToNumber());

					// Todo: Note: This is a special case a model has been loaded by for example ModelLoader::LoadGrid()
					if (path != "Unknown") {
						CRenderable* renderable = actor->AddComponent<CRenderable>();
						renderable->LoadModel(path);
						renderable->SetRenderFlags(renderFlags);
						renderable->SetColor(color);
					}
				}
				else if (name == "CBloomLight") {
					actor->AddComponent<CBloomLight>();
				}
			}

			// Todo: Fix this
			World::Instance().SynchronizeNodeTransforms();
			actor->PostInit();
		}
	}

	void ActorFactory::SaveToFile(std::string filename, const std::vector<SharedPtr<Actor>>& actors)
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

		gLuaManager().GetLuaState()->DumpObject(filename.c_str(), "actor_list", luaScene);
	}

	void ActorFactory::LoadActor(const LuaPlus::LuaObject& luaObject)
	{

	}
}