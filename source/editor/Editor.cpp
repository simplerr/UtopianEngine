#include "Editor.h"
#include "Input.h"
#include "vulkan/Renderer.h"
#include "Camera.h"
#include "core/World.h"
#include "core/components/Actor.h"
#include "core/components/CTransform.h"
#include "core/components/CRenderable.h"
#include "core/components/CLight.h"
#include "vulkan/UIOverlay.h"
#include "editor/ActorInspector.h"
#include "core/terrain/Terrain.h"
#include "editor/TransformTool.h"

namespace Utopian
{
	Editor::Editor(Utopian::Vk::Renderer* renderer, World* world, Terrain* terrain)
		: mRenderer(renderer), mWorld(world), mTerrain(terrain)
	{
		mSelectedActor = nullptr;

		mActorInspector = new ActorInspector();

		mTransformTool = new TransformTool(renderer, mTerrain);

		AddActorCreation("Spot light", ActorTemplate::LIGHT);

		AddPaths();
	}

	Editor::~Editor()
	{
		for (uint32_t i = 0; i < mModelPaths.size(); i++)
			delete mModelPaths[i];

		delete mActorInspector;
		delete mTransformTool;
	}

	void Editor::Update()
	{
		mTransformTool->Update(&gInput(), 0); // Note: Hack

		UpdateUi();

		// Was an Entity selected?
		if (gInput().KeyPressed(VK_LBUTTON) && gInput().KeyDown(VK_LCONTROL))
		{
			Ray ray = mRenderer->GetCamera()->GetPickingRay();

			Actor* selectedActor = mWorld->RayIntersection(ray);
			if (selectedActor != nullptr && selectedActor != mSelectedActor)
			{
				OnActorSelected(selectedActor);
			}
		}

		// Add new model to scene
		if (gInput().KeyPressed('C'))
		{
			SharedPtr<Actor> actor = Actor::Create("EditorActor");

			glm::vec3 pos = glm::vec3(0, 0, 0);
			CTransform* transform = actor->AddComponent<CTransform>(pos);
			CRenderable* renderable = actor->AddComponent<CRenderable>();

			if (mTemplateTypes[mSelectedModel] == ActorTemplate::STATIC_MODEL)
			{
				// Models from adventure_village needs to be scaled and rotated
				transform->SetScale(glm::vec3(50));
				transform->SetRotation(glm::vec3(180, 0, 0));

				renderable->LoadModel(mModelPaths[mSelectedModel]);
			}
			else if (mTemplateTypes[mSelectedModel] == ActorTemplate::LIGHT)
			{
				renderable->LoadModel("data/models/teapot.obj");

				CLight* light = actor->AddComponent<CLight>();
			}

			actor->PostInit();
		}
	}

	void Editor::UpdateUi()
	{
		mActorInspector->UpdateUi();

		if (IsActorSelected())
		{
		}

		// Display Actor creation list
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::SetNextWindowPos(ImVec2(1050, 500));
		ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Actor list", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::PushItemWidth(400.0f);

		ImGui::Text("Models:");

		ImGui::ListBox("", &mSelectedModel, mModelPaths.data(), mModelPaths.size());

		// ImGui functions end here
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void Editor::Draw()
	{
	}

	void Editor::AddActorCreation(std::string path, ActorTemplate actorTemplate)
	{
		mModelPaths.push_back(strdup(path.c_str()));
		mTemplateTypes.push_back(actorTemplate);
	}

	bool Editor::IsActorSelected()
	{
		return mSelectedActor != nullptr;
	}

	void Editor::OnActorSelected(Actor* actor)
	{
		if (IsActorSelected())
		{
			auto renderable = mSelectedActor->GetComponent<CRenderable>();
			renderable->DisableBoundingBox();
		}

		mSelectedActor = actor;

		// Enable bounding box rendering
		auto renderable = mSelectedActor->GetComponent<CRenderable>();
		//renderable->EnableBoundingBox();

		// Create inspector UI
		mActorInspector->SetActor(mSelectedActor);
		mTransformTool->SetActor(mSelectedActor);
	}

	void Editor::AddPaths()
	{
		// Add paths to models that can be loaded
		AddActorCreation("data/models/adventure_village/Barrel.obj");
		AddActorCreation("data/models/adventure_village/Barrel_1.obj");
		AddActorCreation("data/models/adventure_village/CellarEntrance.obj");
		AddActorCreation("data/models/adventure_village/CellarEntrance_1.obj");
		AddActorCreation("data/models/adventure_village/Chimney1.obj");
		AddActorCreation("data/models/adventure_village/Chimney1_1.obj");
		AddActorCreation("data/models/adventure_village/Chimney2.obj");
		AddActorCreation("data/models/adventure_village/Chimney2_1.obj");
		AddActorCreation("data/models/adventure_village/Chimney3.obj");
		AddActorCreation("data/models/adventure_village/Chimney3_1.obj");
		AddActorCreation("data/models/adventure_village/Chimney4.obj");
		AddActorCreation("data/models/adventure_village/Chimney4_1.obj");
		AddActorCreation("data/models/adventure_village/ChimneyBase.obj");
		AddActorCreation("data/models/adventure_village/ChimneyBase_1.obj");
		AddActorCreation("data/models/adventure_village/CrateLong.obj");
		AddActorCreation("data/models/adventure_village/CrateLong_1.obj");
		AddActorCreation("data/models/adventure_village/CrateLongB.obj");
		AddActorCreation("data/models/adventure_village/CrateLongB_1.obj");
		AddActorCreation("data/models/adventure_village/CrateSquare.obj");
		AddActorCreation("data/models/adventure_village/CrateSquare_1.obj");
		AddActorCreation("data/models/adventure_village/CrateSquareB.obj");
		AddActorCreation("data/models/adventure_village/CrateSquareB_1.obj");
		AddActorCreation("data/models/adventure_village/DoorStone.obj");
		AddActorCreation("data/models/adventure_village/DoorStone_1.obj");
		AddActorCreation("data/models/adventure_village/DoorStoneLarge.obj");
		AddActorCreation("data/models/adventure_village/DoorStoneLarge_1.obj");
		AddActorCreation("data/models/adventure_village/DoorWood.obj");
		AddActorCreation("data/models/adventure_village/DoorWood_1.obj");
		AddActorCreation("data/models/adventure_village/ElevatorBeam.obj");
		AddActorCreation("data/models/adventure_village/ElevatorBeam_1.obj");
		AddActorCreation("data/models/adventure_village/HouseAttic.obj");
		AddActorCreation("data/models/adventure_village/HouseAttic_1.obj");
		AddActorCreation("data/models/adventure_village/HouseAtticSmall.obj");
		AddActorCreation("data/models/adventure_village/HouseAtticSmall_1.obj");
		AddActorCreation("data/models/adventure_village/HouseBricksLarge.obj");
		AddActorCreation("data/models/adventure_village/HouseBricksLarge_1.obj");
		AddActorCreation("data/models/adventure_village/HouseBricksNormal.obj");
		AddActorCreation("data/models/adventure_village/HouseBricksNormal_1.obj");
		AddActorCreation("data/models/adventure_village/HouseBricksThin.obj");
		AddActorCreation("data/models/adventure_village/HouseBricksThin_1.obj");
		AddActorCreation("data/models/adventure_village/HouseExtensionRoof.obj");
		AddActorCreation("data/models/adventure_village/HouseExtensionRoof_1.obj");
		AddActorCreation("data/models/adventure_village/HouseStuccoNormal.obj");
		AddActorCreation("data/models/adventure_village/HouseStuccoNormal_1.obj");
		AddActorCreation("data/models/adventure_village/HouseTower.obj");
		AddActorCreation("data/models/adventure_village/HouseTower_1.obj");
		AddActorCreation("data/models/adventure_village/Note.obj");
		AddActorCreation("data/models/adventure_village/PlantA.obj");
		AddActorCreation("data/models/adventure_village/PlantA_1.obj");
		AddActorCreation("data/models/adventure_village/PlantB.obj");
		AddActorCreation("data/models/adventure_village/PlantC.obj");
		AddActorCreation("data/models/adventure_village/PlantC_1.obj");
		AddActorCreation("data/models/adventure_village/PlantC_2.obj");
		AddActorCreation("data/models/adventure_village/PlantD.obj");
		AddActorCreation("data/models/adventure_village/PosterBoard.obj");
		AddActorCreation("data/models/adventure_village/PosterBoard_1.obj");
		AddActorCreation("data/models/adventure_village/SewerDrain.obj");
		AddActorCreation("data/models/adventure_village/SewerDrain_1.obj");
		AddActorCreation("data/models/adventure_village/ShopSign.obj");
		AddActorCreation("data/models/adventure_village/ShopSign_1.obj");
		AddActorCreation("data/models/adventure_village/SignPost.obj");
		AddActorCreation("data/models/adventure_village/SignPost_1.obj");
		AddActorCreation("data/models/adventure_village/StaircaseLarge.obj");
		AddActorCreation("data/models/adventure_village/StaircaseLarge_1.obj");
		AddActorCreation("data/models/adventure_village/StaircaseMedium.obj");
		AddActorCreation("data/models/adventure_village/StaircaseMedium_1.obj");
		AddActorCreation("data/models/adventure_village/StaircaseSmall.obj");
		AddActorCreation("data/models/adventure_village/StaircaseSmall_1.obj");
		AddActorCreation("data/models/adventure_village/StoneBench.obj");
		AddActorCreation("data/models/adventure_village/StoneFence.obj");
		AddActorCreation("data/models/adventure_village/StoneFence_1.obj");
		AddActorCreation("data/models/adventure_village/StonePillar.obj");
		AddActorCreation("data/models/adventure_village/StonePillar_1.obj");
		AddActorCreation("data/models/adventure_village/StonePlatform.obj");
		AddActorCreation("data/models/adventure_village/StonePlatform_1.obj");
		AddActorCreation("data/models/adventure_village/StonePlatform_centered.obj");
		AddActorCreation("data/models/adventure_village/StonePlatformArches.obj");
		AddActorCreation("data/models/adventure_village/StonePlatformArches_1.obj");
		AddActorCreation("data/models/adventure_village/StoneWall.obj");
		AddActorCreation("data/models/adventure_village/StoneWall_1.obj");
		AddActorCreation("data/models/adventure_village/StreetLightSmall.obj");
		AddActorCreation("data/models/adventure_village/StreetLightSmall_1.obj");
		AddActorCreation("data/models/adventure_village/StreetLightTall.obj");
		AddActorCreation("data/models/adventure_village/StreetLightTall_1.obj");
		AddActorCreation("data/models/adventure_village/Tree.obj");
		AddActorCreation("data/models/adventure_village/TreeLog.obj");
		AddActorCreation("data/models/adventure_village/TreeLog_1.obj");
		AddActorCreation("data/models/adventure_village/TreeLogPile.obj");
		AddActorCreation("data/models/adventure_village/TreeLogPile_1.obj");
		AddActorCreation("data/models/adventure_village/TreeLogPile_2.obj");
		AddActorCreation("data/models/adventure_village/TreeLogPile_3.obj");
		AddActorCreation("data/models/adventure_village/Well.obj");
		AddActorCreation("data/models/adventure_village/Well_1.obj");
		AddActorCreation("data/models/adventure_village/Wheel.obj");
		AddActorCreation("data/models/adventure_village/Wheel_1.obj");
		AddActorCreation("data/models/adventure_village/WindowA.obj");
		AddActorCreation("data/models/adventure_village/WindowA_1.obj");
		AddActorCreation("data/models/adventure_village/WindowB.obj");
		AddActorCreation("data/models/adventure_village/WindowB_1.obj");
		AddActorCreation("data/models/adventure_village/WindowC.obj");
		AddActorCreation("data/models/adventure_village/WindowC_1.obj");
		AddActorCreation("data/models/adventure_village/WindowD.obj");
		AddActorCreation("data/models/adventure_village/WindowD_1.obj");
		AddActorCreation("data/models/adventure_village/WindowE.obj");
		AddActorCreation("data/models/adventure_village/WindowE_1.obj");
		AddActorCreation("data/models/adventure_village/WindowF.obj");
		AddActorCreation("data/models/adventure_village/WindowF_1.obj");
		AddActorCreation("data/models/adventure_village/WindowG.obj");
		AddActorCreation("data/models/adventure_village/WindowG_1.obj");
		AddActorCreation("data/models/adventure_village/WoodBench.obj");
		AddActorCreation("data/models/adventure_village/WoodBench_1.obj");
	}
}
