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
#include "core/components/CRigidBody.h"
#include "core/components/CPolyMesh.h"
#include "core/components/CSpawnPoint.h"
#include "vulkan/ModelLoader.h"
#include "core/Log.h"

namespace Utopian
{
	void ActorFactory::LoadFromFile(Window* window, std::string filename)
	{
		UTO_LOG("Loading actors from file");

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
			SceneLayer sceneLayer = (SceneLayer)actorData["scene_layer"].ToInteger();

			SharedPtr<Actor> actor = Actor::Create(name);

			LuaPlus::LuaObject components = actorData["components"];

			// Add Transform component first. This is needed because other components might,
			// depend on it during setup, for example camera->LookAt() needs SynchronizeNodeTransforms() to 
			// be called so that the Camera node can retrieve it's position.
			// Todo: This dependency should be solved in a better way.
			for (LuaPlus::LuaTableIterator luaComponent(components); luaComponent; luaComponent.Next())
			{
				LuaPlus::LuaObject keyComponent = luaComponent.GetKey();
				LuaPlus::LuaObject componentData = luaComponent.GetValue();

				std::string name = keyComponent.ToString();
				if (name == "CTransform")
				{
					glm::vec3 position(componentData["pos_x"].ToNumber(), componentData["pos_y"].ToNumber(), componentData["pos_z"].ToNumber());
					glm::vec3 scale(componentData["scale_x"].ToNumber(), componentData["scale_y"].ToNumber(), componentData["scale_z"].ToNumber());
					glm::quat orientation = glm::quat((float)componentData["orientation_w"].ToNumber(),
													  (float)componentData["orientation_x"].ToNumber(),
													  (float)componentData["orientation_y"].ToNumber(),
													  (float)componentData["orientation_z"].ToNumber());

					CTransform* transform = actor->AddComponent<CTransform>(position);
					transform->SetOrientation(orientation);
					transform->SetScale(scale);
				}
			}

			for (LuaPlus::LuaTableIterator luaComponent(components); luaComponent; luaComponent.Next())
			{
				LuaPlus::LuaObject keyComponent = luaComponent.GetKey();
				LuaPlus::LuaObject componentData = luaComponent.GetValue();

				std::string name = keyComponent.ToString();

				if (name == "CLight")
				{
					glm::vec3 color(componentData["color_r"].ToNumber(), componentData["color_g"].ToNumber(), componentData["color_b"].ToNumber());
					glm::vec3 att(componentData["att_x"].ToNumber(), componentData["att_y"].ToNumber(), componentData["att_z"].ToNumber());
					glm::vec3 dir(componentData["dir_x"].ToNumber(), componentData["dir_y"].ToNumber(), componentData["dir_z"].ToNumber());
					glm::vec3 intensity(componentData["intensity_x"].ToNumber(), componentData["intensity_y"].ToNumber(), componentData["intensity_z"].ToNumber());
					uint32_t type = (uint32_t)componentData["type"].ToInteger();
					float spot = (float)componentData["spot"].ToNumber();
					float range = (float)componentData["range"].ToNumber();

					CLight* light = actor->AddComponent<CLight>();
					light->SetMaterial(glm::vec4(color, 1.0f));
					light->SetDirection(dir);
					light->SetAtt(att.x, att.y, att.z);
					light->SetIntensity(intensity);
					light->SetType((LightType)type);
					light->SetRange(range);
					light->SetSpot(spot);
				}
				else if (name == "CCamera")
				{
					glm::vec3 target(componentData["look_at_x"].ToNumber(), componentData["look_at_y"].ToNumber(), componentData["look_at_z"].ToNumber());
					float fov = (float)componentData["fov"].ToNumber();
					float nearPlane = (float)componentData["near_plane"].ToNumber();
					float farPlane = (float)componentData["far_plane"].ToNumber();
					CCamera* camera = actor->AddComponent<CCamera>(window, fov, nearPlane, farPlane);
					World::Instance().SynchronizeNodeTransforms();
					camera->LookAt(target);
					camera->SetMainCamera(); // Note: Currently this only suppo
				}
				else if (name == "CNoClip")
				{
					float speed = (float)componentData["speed"].ToNumber();
					CNoClip* noclip = actor->AddComponent<CNoClip>(speed);
				}
				else if (name == "CPlayerControl")
				{
					float maxSpeed = (float)componentData["maxSpeed"].ToNumber();
					float jumpStrength = (float)componentData["jumpStrength"].ToNumber();

					CPlayerControl* playerControl = actor->AddComponent<CPlayerControl>(maxSpeed, jumpStrength);
				}
				else if (name == "CRenderable")
				{
					std::string path = componentData["path"].ToString();
					uint32_t renderFlags = (uint32_t)componentData["render_flags"].ToInteger();
					glm::vec4 color(componentData["color_r"].ToNumber(), componentData["color_g"].ToNumber(), componentData["color_b"].ToNumber(), componentData["color_a"].ToNumber());

					CRenderable* renderable = actor->AddComponent<CRenderable>();
					renderable->SetRenderFlags(renderFlags);
					renderable->SetColor(color);

					if (path != "Unknown") {
						renderable->LoadModel(path);
					}
					else {
						renderable->SetModel(Vk::gModelLoader().LoadBox());
					}
					// Todo: Should support loading grids etc. as well.
				}
				else if (name == "CBloomLight")
				{
					actor->AddComponent<CBloomLight>();
				}
				else if (name == "CCatmullSpline")
				{
					std::string filename = componentData["filename"].ToString();
					float timePerSegment = (float)componentData["time_per_segment"].ToNumber();
					uint32_t drawDebug = (uint32_t)componentData["draw_debug"].ToInteger();
					CCatmullSpline* catmullSpline = actor->AddComponent<CCatmullSpline>(filename);
					catmullSpline->SetTimePerSegment(timePerSegment);
					catmullSpline->SetDrawDebug(drawDebug);
				}
				else if (name == "CRigidBody")
				{
					CollisionShapeType collisionShape = (CollisionShapeType)componentData["collisionShapeType"].ToNumber();
					float mass = (float)componentData["mass"].ToNumber();
					float friction = (float)componentData["friction"].ToNumber();
					float rollingFriction = (float)componentData["rollingFriction"].ToNumber();
					float restitution = (float)componentData["restitution"].ToNumber();
					bool kinematic = componentData["kinematic"].GetBoolean();
					glm::vec3 anisotropicFriction(componentData["anisotropic_friciton_x"].ToNumber(),
					 							  componentData["anisotropic_friciton_y"].ToNumber(),
												  componentData["anisotropic_friciton_z"].ToNumber());

					Utopian::CRigidBody* rigidBody = actor->AddComponent<Utopian::CRigidBody>(collisionShape, mass, friction,
																							  rollingFriction, restitution,
																							  kinematic, anisotropicFriction);
				}
				else if (name == "CPolyMesh")
				{
					std::string modelPath = componentData["modelPath"].ToString();
					std::string texturePath = componentData["texturePath"].ToString();
					Utopian::CPolyMesh* polyMesh = actor->AddComponent<Utopian::CPolyMesh>(modelPath, texturePath);
				}
				else if (name == "CSpawnPoint")
				{
					actor->AddComponent<CSpawnPoint>();
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
			if (!actor->ShouldSerialize())
				continue;

			LuaPlus::LuaObject luaActor;
			luaActor.AssignNewTable(gLuaManager().GetLuaState());
			luaActor.SetString("actor_name", actor->GetName().c_str());
			luaActor.SetNumber("scene_layer", (lua_Number)actor->GetSceneLayer());

			LuaPlus::LuaObject luaComponentTable;
			luaComponentTable.AssignNewTable(gLuaManager().GetLuaState());

			std::vector<Component*> components = actor->GetComponents();
			for (auto& component : components)
			{
				std::string name = component->GetName();
				LuaPlus::LuaObject luaObject = component->GetLuaObject();

				if (!luaObject.IsNil())
					luaComponentTable.SetObject(name.c_str(), luaObject);

				// Any extra data required by the component is saved here
				component->SerializeData();
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