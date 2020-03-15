#include "core/World.h"
#include "core/components/Actor.h"
#include "core/components/CRenderable.h"
#include "core/components/CLight.h"
#include "core/components/CCamera.h"
#include "core/components/CTransform.h"
#include "core/AssetLoader.h"
#include "core/ActorFactory.h"
#include "core/ScriptExports.h"
#include "core/LuaManager.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Utopian
{
	World::World()
	{

	}

	World::~World()
	{

	}

	void World::RemoveActor(Actor* actor)
	{
		// Removes the Actor and all of it's components
	}

	SharedPtr<Actor> World::RayIntersection(const Ray& ray)
	{
		SharedPtr<Actor> selectedActor = nullptr;

		float minDistance = FLT_MAX;
		for (auto& actor : mActors)
		{
			if (actor->HasComponent<CRenderable>())
			{
				BoundingBox boundingBox = actor->GetBoundingBox();

				float distance = FLT_MAX;
				if (boundingBox.RayIntersect(ray, distance))// && distance < minDistance)
				{
					selectedActor = actor;
				}
			}
		}

		return selectedActor;
	}

	std::vector<SharedPtr<Actor>>& World::GetActors()
	{
		return mActors;
	}

	uint32_t World::GetActorIndex(SharedPtr<Actor> actor)
	{
		for (uint32_t index = 0; index < mActors.size(); index++)
		{
			if (actor == mActors[index])
				return index;
		}

		assert(0 && "Actor not found");
	}

	void World::BindNode(const SharedPtr<SceneNode>& node, Actor* actor)
	{
		BoundNode binding;
		binding.node = node;
		binding.actor = actor;
		mBoundNodes[node.get()] = binding;
	}

	void World::RemoveNode(const SharedPtr<SceneNode>& node)
	{
		if (mBoundNodes.find(node.get()) != mBoundNodes.end())
		{
			mBoundNodes.erase(node.get());
		}
	}

	void World::SynchronizeNodeTransforms()
	{
		// Synchronize transform between nodes and entities
		for (auto& entry : mBoundNodes)
		{
			entry.second.node->SetTransform(entry.second.actor->GetTransform());
		}
	}

	void World::Update()
	{
		// Loop through actors and check if any should be removed
		for (auto iter = mActors.begin(); iter != mActors.end();)
		{
			SharedPtr<Actor> actor = (*iter);
			if (!actor->IsAlive())
			{
				std::vector<Component*> components = actor->GetComponents();
				for (auto& component : components)
					component->OnDestroyed();

				iter = mActors.erase(iter);
			}
			else
			{
				iter++;
			}
		}

		SynchronizeNodeTransforms();
		
		// Update every active component
		for (auto& entry : mComponents)
		{
			if (entry->IsActive())
			{
				entry->Update();
			}
		}
	}

	void World::RemoveActors()
	{
		for (auto iter = mActors.begin(); iter != mActors.end(); iter++)
		{
			// Todo: Don't remove the camera and light
			if (!(*iter)->HasComponent<CCamera>() && !(*iter)->HasComponent<CLight>())
				(*iter)->SetAlive(false);
		}
	}

	void World::AddActor(const SharedPtr<Actor>& actor)
	{
		mActors.push_back(actor);
	}

	void World::AddComponent(const SharedPtr<Component>& component)
	{
		component->OnCreated();
		mComponents.push_back(component);
	}

	void World::LoadScene()
	{
		// Execute Lua script
		gLuaManager().ExecuteFile("data/scripts/procedural_assets.lua");

		// Note: Todo: This should not be here!!
		ScriptImports::Register();

		LuaPlus::LuaObject object = gLuaManager().GetGlobalVars()["load_foliage"];
		if (object.IsFunction())
		{
			LuaPlus::LuaFunction<int> load_foliage = object;
			load_foliage();
		}

		return;

		SharedPtr<Actor> actor = Actor::Create("Log");

		auto transform = actor->AddComponent<CTransform>(glm::vec3(0, 0, 0));
		transform->SetPosition(glm::vec3(-500.0f, 0, 500));
		transform->SetScale(glm::vec3(0.2f));
		transform->AddRotation(glm::vec3(180.0f, 0, 0));

		auto renderable = actor->AddComponent<CRenderable>();
		renderable->LoadModel("data/NatureManufacture Assets/Meadow Environment Dynamic Nature/Tree Stump/Models/Tree_Stump_01.FBX");

		// Grass assets
		for (uint32_t assetId = 0; assetId < GrassAsset::NUM_GRASS_ASSETS; assetId++)
		{
			actor = Actor::Create("Grass asset");

			transform = actor->AddComponent<CTransform>(glm::vec3(0, 0, 0));
			transform->SetPosition(glm::vec3(0.0f, 0, assetId * 200.0f));
			transform->AddRotation(glm::vec3(180.0f, 0, 0));

			renderable = actor->AddComponent<CRenderable>();
			renderable->SetModel(gAssetLoader().LoadAsset(assetId));
		}

		// Tree assets
		for (uint32_t assetId = POPLAR_PLANT_A_00; assetId < TreeAsset::NUM_TREE_ASSETS; assetId++)
		{
			actor = Actor::Create("Tree asset");

			transform = actor->AddComponent<CTransform>(glm::vec3(0, 0, 0));
			transform->SetPosition(glm::vec3(300.0f, 0, (assetId - POPLAR_PLANT_A_00) * 200.0f));
			transform->AddRotation(glm::vec3(180.0f, 0, 0));
			transform->SetScale(glm::vec3(0.2f));

			renderable = actor->AddComponent<CRenderable>();
			renderable->SetModel(gAssetLoader().LoadAsset(assetId));
		}

		// Rock assets
		for (uint32_t assetId = M_ROCK_01; assetId < RockAsset::NUM_ROCK_ASSETS; assetId++)
		{
			actor = Actor::Create("Tree asset");

			transform = actor->AddComponent<CTransform>(glm::vec3(0, 0, 0));
			transform->SetPosition(glm::vec3(750.0f, 0, (assetId - M_ROCK_01) * 250.0f));
			transform->AddRotation(glm::vec3(180.0f, 0, 0));
			transform->SetScale(glm::vec3(0.10f));

			renderable = actor->AddComponent<CRenderable>();
			renderable->SetModel(gAssetLoader().LoadAsset(assetId));
		}

		// Bush assets
		for (uint32_t assetId = GREY_WILLOW_01; assetId < BushAsset::NUM_BUSH_ASSETS; assetId++)
		{
			actor = Actor::Create("Tree asset");

			transform = actor->AddComponent<CTransform>(glm::vec3(0, 0, 0));
			transform->SetPosition(glm::vec3(1350.0f, 0, (assetId - GREY_WILLOW_01) * 250.0f));
			transform->AddRotation(glm::vec3(180.0f, 0.0f, 0));

			renderable = actor->AddComponent<CRenderable>();
			renderable->SetModel(gAssetLoader().LoadAsset(assetId));
		}

		// Cliff assets
		for (uint32_t assetId = CLIFF_BASE_01; assetId < CliffAsset::NUM_CLIFF_ASSETS; assetId++)
		{
			actor = Actor::Create("Tree asset");

			transform = actor->AddComponent<CTransform>(glm::vec3(0, 0, 0));
			transform->SetPosition(glm::vec3(2050.0f, 0, (assetId - CLIFF_BASE_01) * 650.0f));
			transform->AddRotation(glm::vec3(180.0f, 90.0f, 0));
			transform->SetScale(glm::vec3(0.05f));

			renderable = actor->AddComponent<CRenderable>();
			renderable->SetModel(gAssetLoader().LoadAsset(assetId));
		}
	}
}