#include "Editor.h"
#include "core/Input.h"
#include "vulkan/VulkanApp.h"
#include "core/Camera.h"
#include "core/World.h"
#include "core/Engine.h"
#include "core/components/Actor.h"
#include "core/components/CCamera.h"
#include "core/components/CTransform.h"
#include "core/components/CRenderable.h"
#include "core/components/CLight.h"
#include "core/components/CBloomLight.h"
#include "core/components/CRandomPaths.h"
#include "core/components/CRigidBody.h"
#include "core/components/CSpawnPoint.h"
#include "core/components/CFinishPoint.h"
#include "core/renderer/ImGuiRenderer.h"
#include "vulkan/EffectManager.h"
#include "editor/ActorInspector.h"
#include "editor/TerrainTool.h"
#include "editor/FoliageTool.h"
#include "editor/PrototypeTool.h"
#include "core/ActorFactory.h"
#include "core/renderer/Renderer.h"
#include "core/renderer/Model.h"
#include "core/physics/Physics.h"
#include "utility/math/Helpers.h"
#include "IconFontCppHeaders/IconsFontAwesome4.h"
#include "im3d/im3d.h"
#include "core/Log.h"
#include <imgui/imgui.h>
#include "nativefiledialog/nfd.h"
#include <random>

namespace Utopian
{
   Editor::Editor(ImGuiRenderer* imGuiRenderer, World* world, Terrain* terrain)
      : mImGuiRenderer(imGuiRenderer), mWorld(world), mTerrain(terrain)
   {
      mSelectedActor = nullptr;
      mSelectedActorIndex = 0u;
      mActorInspector = new ActorInspector();

      if (terrain != nullptr)
      {
         mTerrainTool = std::make_shared<TerrainTool>(terrain, gRenderer().GetDevice());
         mFoliageTool = std::make_shared<FoliageTool>(terrain);
         mFoliageTool->SetBrushSettings(mTerrainTool->GetBrushSettings());
      }

      mPrototypeTool = std::make_shared<PrototypeTool>();

      AddActorCreation("Spawn point", ActorTemplate::SPAWN_POINT);
      AddActorCreation("Finish point", ActorTemplate::FINISH_POINT);
      AddActorCreation("Static point light", ActorTemplate::STATIC_POINT_LIGHT);
      AddActorCreation("Physics point light", ActorTemplate::RIGID_SPHERE_LIGHT);

      mSelectedModel = 4; // Physics sphere

      AddPaths();
   }

   Editor::~Editor()
   {
      for (uint32_t i = 0; i < mModelPaths.size(); i++)
         delete mModelPaths[i];

      delete mActorInspector;
   }

   void Editor::PreFrame()
   {
      mPrototypeTool->PreFrame();
   }

   void Editor::Update(double deltaTime)
   {
      UpdateSelectionType();
      DrawGizmo();
      SelectActor();
      AddActorToScene();
      RemoveActorFromScene();
      ScaleSelectedActor();

      if (mTerrain != nullptr)
      {
         mTerrainTool->Update(deltaTime);
         mFoliageTool->Update(deltaTime);
      }

      // Deselect Actor
      if (gInput().KeyPressed(VK_ESCAPE))
      {
         DeselectActor();
         mTerrainTool->DeactivateBrush();
      }

      // Hide/show UI
      if (gInput().KeyPressed('H'))
      {
         if (ImGuiRenderer::GetMode() == UI_MODE_EDITOR)
            ImGuiRenderer::SetMode(UI_MODE_GAME);
         else
            ImGuiRenderer::SetMode(UI_MODE_EDITOR);
      }

      // Recompile shaders
      if (gInput().KeyPressed('R'))
      {
         Vk::gEffectManager().RecompileModifiedShaders();
      }

      mPrototypeTool->Update(mWorld);

      // The UI needs to be updated after updating the selected actor since otherwise
      // the texture descriptor set for the UI can be freed when still being used in a command buffer.
      if (ImGuiRenderer::GetMode() == UI_MODE_EDITOR)
      {
         UpdateUi();

         // Workaround: The real console window is transparent and not supposed to be docked since
         // it also should be displayed when in the game mode. This creates and empty window that
         // take up an area in the dock space. The real console should be placed on top of this window.
         ImGuiRenderer::BeginWindow("Console", glm::vec2(200, 800), 300.0f, ImGuiWindowFlags_NoTitleBar);
         ImGuiRenderer::EndWindow();
      }

      mConsole.Draw("Transparent Console", nullptr);
   }

   void Editor::UpdateSelectionType()
   {
      if (gInput().KeyPressed('G'))
         mSelectionType = OBJECT_SELECTION;
      else if (gInput().KeyPressed('E'))
         mSelectionType = EDGE_SELECTION;
      else if (gInput().KeyPressed('F'))
         mSelectionType = FACE_SELECTION;

      if (gInput().KeyPressed('G') || gInput().KeyPressed('E') || gInput().KeyPressed('F'))
         Im3d::ResetSelectedGizmo();

      mPrototypeTool->SetSelectionType(mSelectionType);
   }
   
   void Editor::DrawGizmo()
   {
      if (mSelectionType == OBJECT_SELECTION)
      {
         if (IsActorSelected() && mSelectedActor->GetComponent<CRenderable>() != nullptr)
         {
            Transform& transform = mSelectedActor->GetTransform();
            Im3d::Mat4 im3dTransform = Im3d::Mat4(transform.GetWorldMatrix());
            if (Im3d::Gizmo("TransformGizmo", im3dTransform))
            {
               transform.SetPosition(im3dTransform.getTranslation());
               transform.SetScale(im3dTransform.getScale());
               transform.SetOrientation(Math::GetQuaternion(im3dTransform));
            }
         }
      }
   }

   void Editor::SelectActor()
   {
      if (gInput().KeyPressed(VK_LBUTTON) && gInput().KeyDown(VK_LCONTROL))
      {
         Ray ray = gRenderer().GetMainCamera()->GetPickingRay();
         IntersectionInfo intersectInfo = mWorld->RayIntersection(ray);

         if (intersectInfo.actor != nullptr)
         {
            mPrototypeTool->ActorSelected(intersectInfo.actor);
            mTerrainTool->DeactivateBrush();

            if (intersectInfo.actor != mSelectedActor)
            {
               mSelectedActorIndex = mWorld->GetActorIndex(intersectInfo.actor);
               OnActorSelected(intersectInfo.actor);
            }
         }
      }
   }
   
   void Editor::AddActorToScene()
   {
      // Copy selected actor (only static models are copied properly)
      if (gInput().KeyPressed('C') && gInput().KeyDown(VK_LCONTROL))
      {
         if (mSelectedActor != nullptr)
         {
            SharedPtr<Actor> newActor = Actor::Create("CopiedActor");

            Transform originalTransform = mSelectedActor->GetTransform();
            glm::vec3 pos = originalTransform.GetPosition();
            glm::quat orientation = originalTransform.GetOrientation();
            glm::vec3 scale = originalTransform.GetScale();

            CTransform* transform = newActor->AddComponent<CTransform>(pos);
            transform->SetOrientation(orientation);
            transform->SetScale(scale);

            std::string originalPath = mSelectedActor->GetComponent<CRenderable>()->GetPath();
            CRenderable* renderable = newActor->AddComponent<CRenderable>();
            renderable->LoadModel(originalPath);

            mSelectedActorIndex = mWorld->GetActorIndex(newActor.get());
            OnActorSelected(newActor.get());
         }
      }
      else if (gInput().KeyPressed('C')) // Add new actor to scene
      {
         Ray ray = gRenderer().GetMainCamera()->GetPickingRay();

         // Shoot rigid bodies but place static objects
         // Todo: Refactor
         glm::vec3 creationPosition;
         bool addActor = false;
         if (mTemplateTypes[mSelectedModel] == RIGID_BOX ||
             mTemplateTypes[mSelectedModel] == RIGID_SPHERE ||
             mTemplateTypes[mSelectedModel] == RIGID_SPHERE_LIGHT)
         {
            float offset = 0.5f;
            creationPosition = gRenderer().GetMainCamera()->GetPosition() + offset * gRenderer().GetMainCamera()->GetDirection();
            addActor = true;
         }
         else
         {
            glm::vec3 intersection = glm::vec3(FLT_MAX);

            IntersectionInfo intersectInfo = mWorld->RayIntersection(ray);

            // Todo: Only check intersection against specific layer
            if (intersectInfo.actor != nullptr)
            {
               intersection = ray.origin + ray.direction * intersectInfo.distance;
               addActor = true;
            }
            else if (mTerrain != nullptr)
            {
               intersection = mTerrain->GetIntersectPoint(ray);
               addActor = true;
            }

            creationPosition = intersection;
         }

         if (addActor)
         {
            CreateActor(mModelPaths[mSelectedModel], mTemplateTypes[mSelectedModel], creationPosition);
         }
      }
   }

   void Editor::RemoveActorFromScene()
   {
      if (gInput().KeyPressed(VK_DELETE) && mSelectedActor != nullptr)
      {
         mSelectedActor->SetAlive(false);
         DeselectActor();
      }
   }

   void Editor::ScaleSelectedActor()
   {
      if (!ImGuiRenderer::IsMouseInsideUi())
      {
         float mouseDz = gInput().MouseDz();
         if (mouseDz != 0.0f && mSelectedActor != nullptr)
         {
            mSelectedActor->GetTransform().AddScale(glm::vec3(mouseDz * MWHEEL_SCALE_FACTOR));
         }
      }
   }

   void Editor::RenderActorCreationUi()
   {
      if (ImGui::CollapsingHeader("Create", ImGuiTreeNodeFlags_DefaultOpen))
      {
         if (ImGui::Button("Load model from file ..."))
         {
            nfdchar_t* modelPath = NULL;
            if (NFD_OpenDialog(NULL, NULL, &modelPath) == NFD_OKAY)
            {
               CreateActor(std::string(modelPath), STATIC_MODEL, glm::vec3(0.0f));
            }
         }

         ImGui::PushItemWidth(ImGui::GetWindowWidth());
         ImGui::Text("Models:");
         ImGui::BeginChild(ImGui::GetID("create_object_scroll"), ImVec2(0, 250));
         for (uint32_t i = 0u; i < mModelPaths.size(); i++)
         {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
               if (i == mSelectedModel)
                  flags |= ImGuiTreeNodeFlags_Selected;

            if (ImGui::TreeNodeEx(mModelPaths[i], flags))
                  ImGui::TreePop();

            if (ImGui::IsItemClicked())
            {
               mSelectedModel = i;
            }
         }
         ImGui::EndChild();
      }
   }

   void Editor::RenderActorSelectionUi()
   {
      if (ImGui::CollapsingHeader("Hierarchy", ImGuiTreeNodeFlags_DefaultOpen))
      {
         ImGui::PushItemWidth(ImGui::GetWindowWidth());
         std::vector<SharedPtr<Actor>>& actors = World::Instance().GetActors();

         ImGui::BeginChild(ImGui::GetID("scene_hierarchy_scroll"), ImVec2(0, 450));
         if (ImGui::TreeNodeEx("Root", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth))
         {
            for (auto actor : actors)
            {
               std::string name = actor->GetName();

               ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
               if (mSelectedActor == actor.get())
                  flags = ImGuiTreeNodeFlags_Selected;

               std::string icon = ICON_FA_CUBE;

               if (actor->HasComponent<CLight>())
                  icon = ICON_FA_LIGHTBULB_O;
               else if (actor->HasComponent<CCamera>())
                  icon = ICON_FA_CAMERA;
                 
               if (ImGui::TreeNodeEx(actor.get(), flags, (icon + " " + name).c_str()))
                  ImGui::TreePop();

               if (ImGui::IsItemClicked())
               {
                  OnActorSelected(actor.get());
               }
            }
            ImGui::TreePop();
         }
         ImGui::EndChild();
      }
   }

   void Editor::RenderLoadSaveUi()
   {
      auto clearActors = [&]()
      {
         World::Instance().RemoveActors();
         DeselectActor();
      };

      if (ImGui::CollapsingHeader("Load and save", ImGuiTreeNodeFlags_DefaultOpen))
      {
         if (ImGui::Button("Save scene"))
            ActorFactory::SaveToFile("data/scene.lua", World::Instance().GetActors());

         ImGui::SameLine();

         if (ImGui::Button("Save scene as ..."))
         {
            nfdchar_t* scenePath = NULL;
            if (NFD_SaveDialog(NULL, NULL, &scenePath))
               ActorFactory::SaveToFile(std::string(scenePath), World::Instance().GetActors());
         }

         if (ImGui::Button("Load scene"))
         {
            clearActors();
            ActorFactory::LoadFromFile(Utopian::gEngine().GetVulkanApp()->GetWindow(), "data/scene.lua");
         }

         ImGui::SameLine();

         if (ImGui::Button("Load scene ..."))
         {
            nfdchar_t* scenePath = NULL;
            if (NFD_OpenDialog(NULL, NULL, &scenePath) == NFD_OKAY)
            {
               clearActors();
               ActorFactory::LoadFromFile(Utopian::gEngine().GetVulkanApp()->GetWindow(), std::string(scenePath));
            }
         }

         ImGui::SameLine();

         if (ImGui::Button("Clear scene"))
            clearActors();

         if (ImGui::Button("Save terrain"))
         {
            gRenderer().SaveInstancesToFile("data/instances.txt");

            if(mTerrain != nullptr)
            {
               mTerrain->SaveHeightmap("data/heightmap.ktx");
               mTerrain->SaveBlendmap("data/blendmap.ktx");
            }
         }

         ImGui::SameLine();

         if (ImGui::Button("Reload foliage"))
         {
            World::Instance().RemoveActors();
            World::Instance().LoadProceduralAssets();
         }
      }
   }

   void Editor::UpdateUi()
   {
      // UI containing settings, terrain and foliage tools
      ImGuiRenderer::BeginWindow("Terrain", glm::vec2(200, 800), 300.0f);
      {
         bool physicsEnabled = gPhysics().IsEnabled();
         bool debugDrawEnabled = gPhysics().IsDebugDrawEnabled();
         ImGui::Checkbox("Simulate physics", &physicsEnabled);
         ImGui::Checkbox("Physics debug draw", &debugDrawEnabled);
         gPhysics().EnableSimulation(physicsEnabled);
         gPhysics().EnableDebugDraw(debugDrawEnabled);

         ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.70f);

         if (mTerrain != nullptr)
         {
            BrushSettings::Mode previousMode = mTerrainTool->GetBrushMode();

            mTerrainTool->RenderUi();
            mFoliageTool->RenderUi();

            if (previousMode == BrushSettings::Mode::INACTIVE &&
                mTerrainTool->GetBrushMode() != BrushSettings::Mode::INACTIVE)
            {
               DeselectActor();
            }
         }

         mPrototypeTool->RenderUi();
      }
      ImGuiRenderer::EndWindow();

      ImGuiRenderer::BeginWindow("Scene", glm::vec2(200, 800), 300.0f);

      RenderActorCreationUi();
      RenderActorSelectionUi();
      RenderLoadSaveUi();

      ImGuiRenderer::EndWindow();

      // Needs to be called after RenderActorSelectionUi() otherwhise the UI textures
      // will be in use in the ImGuiRenderer command queue.
      // Todo: Fix. Issue #91.
      mActorInspector->UpdateUi();

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

   void Editor::OnActorSelected(Actor* actor)
   {
      if (IsActorSelected())
      {
         auto renderable = mSelectedActor->GetComponent<CRenderable>();
         if (renderable != nullptr)
         {
            renderable->DisableBoundingBox();
            renderable->RemoveRenderFlags(RENDER_FLAG_DRAW_OUTLINE);
         }
      }

      mSelectedActor = actor;

      if (mSelectedActor != nullptr)
      {
         UTO_LOG("Actor \"" + mSelectedActor->GetName() + "\" selected, ID: " + std::to_string(mSelectedActor->GetId()));

         // Enable outline rendering
         auto renderable = mSelectedActor->GetComponent<CRenderable>();
         if (renderable != nullptr)
            renderable->AppendRenderFlags(RENDER_FLAG_DRAW_OUTLINE);
      }

      // Create inspector UI
      mActorInspector->SetActor(mSelectedActor);
   }

   void Editor::DeselectActor()
   {
      OnActorSelected(nullptr);
      mPrototypeTool->ActorSelected(nullptr);
   }

   void Editor::CreateActor(std::string modelPath, ActorTemplate actorTemplate, glm::vec3 position)
   {
      std::string name = modelPath.substr(modelPath.find_last_of("/") + 1);
      name = name.substr(name.find_last_of("\\") + 1);

      SharedPtr<Actor> actor = Actor::Create(name);
      CTransform* transform = actor->AddComponent<CTransform>(position);
      CRenderable* renderable = actor->AddComponent<CRenderable>();

      /*
      * Todo:
      * These are just some example templates for creating actors
      * It will be extended so that new actor templates can be 
      * configured completly in Lua and show up in the editor.
      */
      if (actorTemplate == ActorTemplate::STATIC_MODEL)
      {
         transform->AddRotation(glm::vec3(glm::pi<float>(), 0, 0));

         CRigidBody* rigidBody = actor->AddComponent<CRigidBody>();

         renderable->LoadModel(modelPath);
      }
      else if (actorTemplate == ActorTemplate::STATIC_POINT_LIGHT)
      {
         renderable->LoadModel("data/models/teapot.obj");
         renderable->SetRenderFlags(RenderFlags::RENDER_FLAG_COLOR);

         CLight* light = actor->AddComponent<CLight>();
         CBloomLight* bloomLight = actor->AddComponent<CBloomLight>();

         glm::vec4 color = glm::vec4(Math::GetRandomVec3(1.0f, 1.0f), 0.0f);
         light->SetColor(color);
         renderable->SetColor(glm::vec4(color.r, color.g, color.g, 2.4));
      }
      else if (actorTemplate == ActorTemplate::RIGID_BOX)
      {
         CRigidBody* rigidBody = actor->AddComponent<CRigidBody>();
         rigidBody->SetCollisionShapeType(CollisionShapeType::BOX);

         renderable->LoadModel(modelPath);
      }
      else if (actorTemplate == ActorTemplate::RIGID_SPHERE)
      {
         float scale = Math::GetRandom(20.0, 20.0f);

         transform->SetScale(glm::vec3(scale));

         CRigidBody* rigidBody = actor->AddComponent<CRigidBody>();
         rigidBody->SetCollisionShapeType(CollisionShapeType::SPHERE);
         renderable->LoadModel(modelPath);
         renderable->SetPushFoliage(true);
      }
      else if (actorTemplate == ActorTemplate::RIGID_SPHERE_LIGHT)
      {
         transform->SetScale(glm::vec3(20));

         CRigidBody* rigidBody = actor->AddComponent<CRigidBody>();
         rigidBody->SetCollisionShapeType(CollisionShapeType::SPHERE);
         renderable->LoadModel("data/models/sphere_lowres.obj");
         renderable->SetPushFoliage(true);
         renderable->SetRenderFlags(RenderFlags::RENDER_FLAG_COLOR);

         // Copy paste from static light
         CLight* light = actor->AddComponent<CLight>();
         CBloomLight* bloomLight = actor->AddComponent<CBloomLight>();

         glm::vec4 color = glm::vec4(Math::GetRandomVec3(1.0f, 1.0f), 0.0f);
         light->SetColor(color);
         renderable->SetColor(glm::vec4(color.r, color.g, color.g, 2.4));
      }
      else if (actorTemplate == ActorTemplate::SPAWN_POINT)
      {
         transform->AddRotation(glm::vec3(glm::pi<float>(), 0, 0));
         renderable->LoadModel("data/models/spawn_cone.fbx");
         actor->AddComponent<CSpawnPoint>();
      }
      else if (actorTemplate == ActorTemplate::FINISH_POINT)
      {
         renderable->LoadModel("data/models/spawn_cylinder.fbx");
         actor->AddComponent<CFinishPoint>();
      }
      else if (actorTemplate == ActorTemplate::PBR_SPHERE)
      {
         transform->AddRotation(glm::vec3(glm::pi<float>(), 0, 0));

         CRigidBody* rigidBody = actor->AddComponent<CRigidBody>();

         renderable->LoadModel(modelPath);

         Utopian::Model* model = renderable->GetInternal()->GetModel();
         Utopian::Material* material = model->GetMaterial(0);
         material->properties->data.baseColorFactor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
         material->properties->data.metallicFactor = 1.0f;
         material->properties->data.roughnessFactor = 0.0;
         material->properties->data.occlusionFactor = 0.5;
         material->properties->UpdateMemory();
      }

      World::Instance().SynchronizeNodeTransforms();
      actor->PostInit();

      CRigidBody* rigidBody = actor->GetComponent<CRigidBody>();
      if (rigidBody != nullptr)
      {
         const float impulse = 10.0f;
         rigidBody->ApplyCentralImpulse(gRenderer().GetMainCamera()->GetDirection() * impulse);

         // Hack:
         if (actorTemplate == ActorTemplate::STATIC_MODEL)
         {
            // Must be called after PostInit() since it needs the Renderable component
            rigidBody->SetKinematic(true);
         }
      }

      OnActorSelected(actor.get());
   }

   void Editor::AddPaths()
   {
      // Add paths to models that can be loaded
      AddActorCreation("data/models/gltf/Fox/glTF/Fox.gltf");
      AddActorCreation("data/models/gltf/CesiumMan.gltf");
      AddActorCreation("data/models/gltf/FlightHelmet/glTF/FlightHelmet.gltf");
      AddActorCreation("data/models/gltf/DamagedHelmet/glTF/DamagedHelmet.gltf");
      AddActorCreation("data/models/gltf/sphere.gltf", ActorTemplate::PBR_SPHERE);
      AddActorCreation("data/models/sheep/sheep.obj");
      AddActorCreation("data/models/adventure_village/CrateLong_reflective.obj");
      AddActorCreation("data/models/sphere_lowres.obj", ActorTemplate::RIGID_SPHERE);
      AddActorCreation("data/models/adventure_village/CrateLong.obj", ActorTemplate::RIGID_BOX);
      AddActorCreation("data/models/adventure_village/Barrel.obj", ActorTemplate::RIGID_BOX);
      AddActorCreation("data/models/adventure_village/Barrel_1.obj", ActorTemplate::RIGID_BOX);
      AddActorCreation("data/models/sponza/sponza.obj");
      AddActorCreation("data/models/fps_hands/hands.obj");
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
