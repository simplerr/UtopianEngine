#include "Editor.h"
#include "Input.h"
#include "vulkan/VulkanApp.h"
#include "Camera.h"
#include "core/World.h"
#include "core/components/Actor.h"
#include "core/components/CTransform.h"
#include "core/components/CRenderable.h"
#include "core/components/CLight.h"
#include "core/components/CBloomLight.h"
#include "core/components/CRandomPaths.h"
#include "core/components/CRigidBody.h"
#include "ImGuiRenderer.h"
#include "vulkan/EffectManager.h"
#include "editor/ActorInspector.h"
#include "core/legacy/BaseTerrain.h"
#include "editor/TransformTool.h"
#include "editor/TerrainTool.h"
#include "editor/FoliageTool.h"
#include "core/ActorFactory.h"
#include "core/renderer/Renderer.h"
#include "core/physics/Physics.h"
#include <random>

namespace Utopian
{
	Editor::Editor(ImGuiRenderer* imGuiRenderer, Camera* camera, World* world, const SharedPtr<Terrain>& terrain)
		: mImGuiRenderer(imGuiRenderer), mCamera(camera), mWorld(world), mTerrain(terrain)
	{
		mSelectedActor = nullptr;
		mActorInspector = new ActorInspector();
		mTransformTool = std::make_shared<TransformTool>(mTerrain, camera);
		mTerrainTool = std::make_shared<TerrainTool>(terrain, gRenderer().GetDevice());
		mFoliageTool = std::make_shared<FoliageTool>(terrain, gRenderer().GetDevice());
		mFoliageTool->SetBrushSettings(mTerrainTool->GetBrushSettings());

		AddActorCreation("Static point light", ActorTemplate::STATIC_POINT_LIGHT);
		AddActorCreation("Moving point light", ActorTemplate::MOVING_POINT_LIGHT);

		mSelectedModel = 16;

		AddPaths();
	}

	Editor::~Editor()
	{
		for (uint32_t i = 0; i < mModelPaths.size(); i++)
			delete mModelPaths[i];

		delete mActorInspector;
	}

	void Editor::Update()
	{
		mTransformTool->Update(&gInput(), 0); // Note: Hack
		mTerrainTool->Update();
		mFoliageTool->Update();

		UpdateUi();

		// Was an Entity selected?
		if (gInput().KeyPressed(VK_LBUTTON) && gInput().KeyDown(VK_LCONTROL))
		{
			Ray ray = mCamera->GetPickingRay();

			Actor* selectedActor = mWorld->RayIntersection(ray);
			if (selectedActor != nullptr && selectedActor != mSelectedActor)
			{
				OnActorSelected(selectedActor);
			}
		}

		// Add new actor to scene
		if (gInput().KeyPressed('C'))
		{
			Ray ray = gRenderer().GetMainCamera()->GetPickingRay();
			glm::vec3 intersection  = mTerrain->GetIntersectPoint(ray);

			SharedPtr<Actor> actor = Actor::Create("EditorActor");

			glm::vec3 pos = intersection + glm::vec3(0, 50.0f, 0);
			pos.y = -900;
			CTransform* transform = actor->AddComponent<CTransform>(pos);
			CRenderable* renderable = actor->AddComponent<CRenderable>();

			if (mTemplateTypes[mSelectedModel] == ActorTemplate::STATIC_MODEL)
			{
				// Models from adventure_village needs to be scaled and rotated
				transform->SetScale(glm::vec3(50));
				//transform->SetRotation(glm::vec3(180, 0, 0));

				// Temporary physics testing:
				transform->AddTranslation(glm::vec3(0.0f, 200.0f, 0.0f));
				CRigidBody* rigidBody = actor->AddComponent<CRigidBody>();

				renderable->LoadModel(mModelPaths[mSelectedModel]);
			}
			else if (mTemplateTypes[mSelectedModel] == ActorTemplate::STATIC_POINT_LIGHT)
			{
				renderable->LoadModel("data/models/teapot.obj");
				renderable->SetRenderFlags(RenderFlags::RENDER_FLAG_COLOR);

				CLight* light = actor->AddComponent<CLight>();
				CBloomLight* bloomLight = actor->AddComponent<CBloomLight>();

				// Set random light color
				std::random_device rd;
				std::mt19937 mt(rd());
				std::uniform_real_distribution<double> dist(0.0, 1.0);
				glm::vec4 color = glm::vec4(dist(mt), dist(mt), dist(mt), 0.0f);
				color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f); // White for now
				light->SetMaterial(color);
				renderable->SetColor(glm::vec4(color.r, color.g, color.g, 2.4)); // Set brightness
			}
			else if (mTemplateTypes[mSelectedModel] == ActorTemplate::MOVING_POINT_LIGHT)
			{
				renderable->LoadModel("data/models/teapot.obj");
				renderable->SetRenderFlags(RenderFlags::RENDER_FLAG_COLOR);

				CLight* light = actor->AddComponent<CLight>();
				CBloomLight* bloomLight = actor->AddComponent<CBloomLight>();
				CRandomPaths* randomPaths = actor->AddComponent<CRandomPaths>(mTerrain);

				// Set random light color
				std::random_device rd;
				std::mt19937 mt(rd());
				std::uniform_real_distribution<double> dist(0.0, 1.0);
				glm::vec4 color = glm::vec4(dist(mt), dist(mt), dist(mt), 0.0f);
				color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f); // White for now
				light->SetMaterial(color);
				renderable->SetColor(glm::vec4(color.r, color.g, color.g, 2.4)); // Set brightness
			}

			World::Instance().SynchronizeNodeTransforms();
			actor->PostInit();
		}

		// Remove actor from scene
		if (gInput().KeyPressed(VK_DELETE) && mSelectedActor != nullptr)
		{
			mSelectedActor->SetAlive(false);
			UnselectActor();
		}

		// Hide/show UI
		if (gInput().KeyPressed('H'))
		{
			mImGuiRenderer->ToggleVisible();
		}

		// Recompile shaders
		if (gInput().KeyPressed('R'))
		{
			Vk::gEffectManager().RecompileModifiedShaders();
		}
	}

	void Editor::RenderActorCreationUi()
	{
		// Display Actor creation list
		if (ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("Models:");
			ImGui::ListBox("", &mSelectedModel, mModelPaths.data(), mModelPaths.size());

			if (ImGui::Button("Save scene"))
				ActorFactory::SaveToFile("data/scene.lua", World::Instance().GetActors());

			if (ImGui::Button("Clear scene"))
				World::Instance().RemoveActors();

			if (ImGui::Button("Reload foliage"))
			{
				World::Instance().RemoveActors();
				World::Instance().LoadScene();
			}

			if (ImGui::Button("Reload scene"))
			{
				World::Instance().RemoveActors();
				World::Instance().LoadScene();
			}
		}
	}

	void Editor::UpdateUi()
	{
		mActorInspector->UpdateUi();

		// UI containing settings, terrain and foliage tools
		ImGuiRenderer::BeginWindow("Editor", glm::vec2(200, 800), 300.0f);

		bool physicsEnabled = gPhysics().IsEnabled();
		bool debugDrawEnabled = gPhysics().IsDebugDrawEnabled();
		ImGui::Checkbox("Simulate physics", &physicsEnabled);
		ImGui::Checkbox("Physics debug draw", &debugDrawEnabled);
		gPhysics().EnableSimulation(physicsEnabled);
		gPhysics().EnableDebugDraw(debugDrawEnabled);

		mTerrainTool->RenderUi();
		mFoliageTool->RenderUi();
		RenderActorCreationUi();

		ImGuiRenderer::EndWindow();

		ImGuiRenderer::BeginWindow("Effects", glm::vec2(10, 800), 300.0f);

		if (ImGui::Button("Recompile modified shaders"))
		{
			Vk::gEffectManager().RecompileModifiedShaders();
		}

		if (ImGui::Button("Recompile all shaders"))
		{
			Vk::gEffectManager().RecompileAllShaders();
		}

		ImGuiRenderer::EndWindow();
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

	void Editor::UnselectActor()
	{
		mSelectedActor = nullptr;
		mTransformTool->SetActor(nullptr);
		mActorInspector->SetActor(nullptr);
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
