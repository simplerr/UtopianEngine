#include "utopian/core/renderer/ImGuiRenderer.h"
#include "utopian/vulkan/Debug.h"
#include "utopian/core/components/CTransform.h"
#include "utopian/core/components/CLight.h"
#include "utopian/core/components/CRenderable.h"
#include "utopian/core/components/CRigidBody.h"
#include "utopian/core/components/CCatmullSpline.h"
#include "utopian/core/components/CPolyMesh.h"
#include "utopian/core/components/CPlayerControl.h"
#include "utopian/core/components/CNoClip.h"
#include "utopian/core/components/Actor.h"
#include "utopian/core/Log.h"
#include "utopian/core/Engine.h"
#include "utopian/vulkan/Texture.h"
#include "utopian/vulkan/TextureLoader.h"
#include "utopian/vulkan/handles/Image.h"
#include "utopian/core/renderer/Renderer.h"
#include "utopian/core/renderer/Model.h"
#include "ComponentInspector.h"
#include <nativefiledialog/nfd.h>
#include <glm/gtc/quaternion.hpp>
#include <algorithm>
#include <imgui/imgui.h>

namespace Utopian
{
   ComponentInspector::ComponentInspector()
   {
   }

   ComponentInspector::~ComponentInspector()
   {

   }

   TransformInspector::TransformInspector(CTransform* transform)
   {
      mComponent = transform;
      mTransform = mComponent->GetTransform();
   }

   void TransformInspector::UpdateUi()
   {
      mTransform = mComponent->GetTransform();

      if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
      {
         ImGuiRenderer::DrawVec3Control("Position", mTransform.mPosition);
         ImGuiRenderer::DrawVec3Control("Scale", mTransform.mScale);

         glm::vec3 rotation = glm::vec3(0.0f);
         ImGuiRenderer::DrawVec3Control("Rotation", rotation);

         static bool localRotation = true;
         ImGui::Checkbox("Local rotation", &localRotation);

         mComponent->SetPosition(mTransform.mPosition);
         mComponent->SetScale(mTransform.GetScale());
         mComponent->AddRotation(rotation, localRotation);

         ImGui::Separator();
      }
   }

   RenderableInspector::ModelInspector::ModelInspector(Utopian::Model* model)
   {
      SetModel(model);
      selectedMaterial = 0;
   }

   RenderableInspector::ModelInspector::~ModelInspector()
   {
      ClearTextures();
   }

   void RenderableInspector::ModelInspector::SetModel(Utopian::Model* model)
   {
      ClearTextures();

      this->model = model;
      AddTextures(0);
   }

   void RenderableInspector::ModelInspector::UpdateUi()
   {
      if (ImGui::CollapsingHeader("Primitives"))
      {
         for (uint32_t index = 0; index < model->GetNumPrimitives(); index++)
         {
            Utopian::Primitive* primitive = model->GetPrimitive(index);

            if (ImGui::TreeNodeEx(primitive->GetDebugName().c_str()))
            {
               ImGui::Text("Num vertices: %u", primitive->GetNumVertices());
               ImGui::Text("Num indices: %u", primitive->GetNumIndices());
               ImGui::TreePop();
            }
         }
      }

      if (ImGui::CollapsingHeader("Materials"))
      {
         for (uint32_t index = 0; index < model->GetNumMaterials(); index++)
         {
            Utopian::Material* material = model->GetMaterial(index);

            ImGuiTreeNodeFlags flags = 0;
            if (selectedMaterial == index)
               flags = ImGuiTreeNodeFlags_Selected;

            if (ImGui::TreeNodeEx(material->name.c_str(), flags))
            {
               ImGui::TreePop();
            }

            if (ImGui::IsItemClicked())
            {
               selectedMaterial = index;
               ClearTextures();
               AddTextures(selectedMaterial);
            }
         }
      }

      if (model->IsAnimated())
      {
         if (ImGui::CollapsingHeader("Animations"))
         {
            SkinAnimator* animator = model->GetAnimator();

            bool paused = animator->GetPaused();
            ImGui::Checkbox("Paused", &paused);
            animator->SetPaused(paused);

            std::string uiString = "";
            for (uint32_t i = 0; i < animator->GetNumAnimations(); i++)
               uiString += animator->GetAnimationName(i) + '\0';

            int activeAnimation = animator->GetActiveAnimation();
            if (ImGui::Combo("Active animation", &activeAnimation, uiString.c_str()))
            {
               animator->SetAnimation(activeAnimation);
            }
         }
      }

      // Since it's inside the ImGui::ImageButton we are changing the texture
      // we cannot change the UI texture immedietly has that would make the
      // descriptor set used in ImGuiRenderer::UpdateCommandBuffers() invalid.
      static bool changedTexture = false;

      if (changedTexture) {
         ClearTextures();
         AddTextures(selectedMaterial);
      }

      changedTexture = false;

      if (ImGui::CollapsingHeader("Selected material", ImGuiTreeNodeFlags_DefaultOpen))
      {
         Material* material = model->GetMaterial(selectedMaterial);

         ImGui::Text("Name: %s", material->name.c_str());
         ImGui::ColorEdit4("Base color", &material->properties->data.baseColorFactor.x);
         ImGui::SliderFloat("Metallic", &material->properties->data.metallicFactor, 0.0, 1.0f);
         ImGui::SliderFloat("Roughness", &material->properties->data.roughnessFactor, 0.0, 1.0f);
         ImGui::SliderFloat("Ambient occlusion", &material->properties->data.occlusionFactor, 0.0, 1.0f);
         material->properties->UpdateMemory();

         if (ImGui::CollapsingHeader("Textures"))
         {
            for (uint32_t i = 0; i < textureInfos.size(); i++)
            {
               TextureInfo textureInfo = textureInfos[i];
               ImGui::Text(textureInfo.texture->GetPath().c_str());
               if (ImGui::ImageButton(textureInfo.textureId, ImVec2(64, 64)))
               {
                  nfdchar_t* outPath = NULL;
                  nfdresult_t result = NFD_OpenDialog(NULL, NULL, &outPath);
                  SharedPtr<Vk::Texture> loadedTexture = Vk::gTextureLoader().LoadTexture(std::string(outPath));
                  //textureInfo.texture = Vk::gTextureLoader().LoadTexture(std::string(outPath));
                  switch (i) {
                     case 0: material->colorTexture = loadedTexture; break;
                     case 1: material->normalTexture = loadedTexture; break;
                     case 2: material->specularTexture = loadedTexture; break;
                     case 3: material->metallicRoughnessTexture = loadedTexture; break;
                     case 4: material->occlusionTexture = loadedTexture; break;
                  }
                  
                  material->UpdateTextureDescriptors(Utopian::gEngine().GetVulkanApp()->GetDevice());
                  changedTexture = true;
               }
            }
         }
      }
   }

   void RenderableInspector::ModelInspector::ClearTextures()
   {
      for (auto& textureInfo : textureInfos)
      {
         gEngine().GetImGuiRenderer()->FreeTexture(textureInfo.textureId);
      }

      textureInfos.clear();
   }

   void RenderableInspector::ModelInspector::AddTextures(uint32_t materialIndex)
   {
      Material* material = model->GetMaterial(materialIndex);
      Utopian::ImGuiRenderer* ui = gEngine().GetImGuiRenderer();

      textureInfos.push_back(TextureInfo(material->colorTexture));
      textureInfos.push_back(TextureInfo(material->normalTexture));
      textureInfos.push_back(TextureInfo(material->specularTexture));
      textureInfos.push_back(TextureInfo(material->metallicRoughnessTexture));
      textureInfos.push_back(TextureInfo(material->occlusionTexture));
   }

   RenderableInspector::ModelInspector::TextureInfo::TextureInfo(SharedPtr<Vk::Texture> texture)
   {
      Utopian::ImGuiRenderer* ui = gEngine().GetImGuiRenderer();

      this->textureId = ui->AddImage(texture->GetImage());
      this->texture = texture;
   }

   RenderableInspector::RenderableInspector(CRenderable* renderable)
   {
      mRenderable = renderable;
      mTextureTiling = renderable->GetTextureTiling().x;
      mDeferred = renderable->HasRenderFlags(RenderFlags::RENDER_FLAG_DEFERRED);
      mColor = renderable->HasRenderFlags(RenderFlags::RENDER_FLAG_COLOR);
      mBoundingBox = renderable->HasRenderFlags(RenderFlags::RENDER_FLAG_BOUNDING_BOX);
      mDebugNormals = renderable->HasRenderFlags(RenderFlags::RENDER_FLAG_NORMAL_DEBUG);
      mWireframe = renderable->HasRenderFlags(RenderFlags::RENDER_FLAG_WIREFRAME);
      mCastShadow = renderable->HasRenderFlags(RenderFlags::RENDER_FLAG_CAST_SHADOW);
      mVisible = renderable->IsVisible();
      mModelInspector = std::make_shared<ModelInspector>(renderable->GetModel());
   }

   RenderableInspector::~RenderableInspector()
   {
   }

   void RenderableInspector::UpdateUi()
   {
      if (ImGui::CollapsingHeader("Renderable", ImGuiTreeNodeFlags_DefaultOpen))
      {
         if (ImGui::Checkbox("Visible", &mVisible))
         {
            mRenderable->SetVisible(mVisible);
         }

         if (ImGui::Checkbox("Deferred", &mDeferred))
         {
            uint32_t flag = mRenderable->GetRenderFlags();

            if (mDeferred)
               flag |= RenderFlags::RENDER_FLAG_DEFERRED;
            else
               flag &= ~RenderFlags::RENDER_FLAG_DEFERRED;

            mRenderable->SetRenderFlags(flag);
         }

         if (ImGui::Checkbox("Color", &mColor))
         {
            uint32_t flag = mRenderable->GetRenderFlags();

            if (mColor)
               flag |= RenderFlags::RENDER_FLAG_COLOR;
            else
               flag &= ~RenderFlags::RENDER_FLAG_COLOR;

            mRenderable->SetRenderFlags(flag);
         }

         if (ImGui::Checkbox("Bounding box", &mBoundingBox))
         {
            uint32_t flag = mRenderable->GetRenderFlags();

            if (mBoundingBox)
               flag |= RenderFlags::RENDER_FLAG_BOUNDING_BOX;
            else
               flag &= ~RenderFlags::RENDER_FLAG_BOUNDING_BOX;

            mRenderable->SetRenderFlags(flag);
         }

         if (ImGui::Checkbox("Debug normals", &mDebugNormals))
         {
            uint32_t flag = mRenderable->GetRenderFlags();

            if (mDebugNormals)
               flag |= RenderFlags::RENDER_FLAG_NORMAL_DEBUG;
            else
               flag &= ~RenderFlags::RENDER_FLAG_NORMAL_DEBUG;

            mRenderable->SetRenderFlags(flag);
         }

         if (ImGui::Checkbox("Wireframe", &mWireframe))
         {
            uint32_t flag = mRenderable->GetRenderFlags();

            if (mWireframe)
               flag |= RenderFlags::RENDER_FLAG_WIREFRAME;
            else
               flag &= ~RenderFlags::RENDER_FLAG_WIREFRAME;

            mRenderable->SetRenderFlags(flag);
         }

         if (ImGui::Checkbox("Cast shadow", &mCastShadow))
         {
            uint32_t flag = mRenderable->GetRenderFlags();

            if (mCastShadow)
               flag |= RenderFlags::RENDER_FLAG_CAST_SHADOW;
            else
               flag &= ~RenderFlags::RENDER_FLAG_CAST_SHADOW;

            mRenderable->SetRenderFlags(flag);
         }

         // If the renderable has a light then let the light inspector control the color
         glm::vec4 color = mRenderable->GetColor();
         if (!mRenderable->GetParent()->HasComponent<CLight>())
         {
            ImGui::ColorEdit3("Color", &color.x);
         }

         ImGui::SliderFloat("Brightness", &color.w, 0.0f, 100.0, "%.1f");
         mRenderable->SetColor(color);

         ImGui::SliderFloat("Tiling", &mTextureTiling, 0.5, 50);
         mRenderable->SetTileFactor(glm::vec2(mTextureTiling));

         // Displays UI elements for all primitives and materials in a model
         mModelInspector->UpdateUi();

         ImGui::Separator();
      }
   }

   LightInspector::LightInspector(CLight* light)
   {
      mLight = light;
      mLightData = mLight->GetLightData();
      mType = light->GetLightType();
   }

   void LightInspector::UpdateUi()
   {
      if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
      {
         ImGui::Combo("Type", &mType, "Directional\0Point\0Spot\0");

         ImGui::SliderFloat3("Intensity", &mLightData.intensity.x, 0.0f, 10.0f);
         ImGui::SliderFloat3("Direction", &mLightData.direction.x, -1.0f, 1.0f);
         ImGui::SliderFloat3("Attenuation", &mLightData.att.x, 0.0f, 3.0f);
         ImGui::SliderFloat("Range", &mLightData.range, 0.0f, 10000.0f);
         ImGui::SliderFloat("Spot", &mLightData.spot, 0.0f, 30.0f);
         ImGui::ColorEdit3("Color", &mLightData.color.x);

         mLight->SetIntensity(mLightData.intensity);
         mLight->SetDirection(mLightData.direction);
         mLight->SetColor(mLightData.color);
         mLight->SetRange(mLightData.range);
         mLight->SetSpot(mLightData.spot);
         mLight->SetAttenuation(mLightData.att);
         mLight->SetType((Utopian::LightType)mType);

         ImGui::Separator();
      }
   }

   RigidBodyInspector::RigidBodyInspector(CRigidBody* rigidBody)
   {
      mRigidBody = rigidBody;
   }

   void RigidBodyInspector::UpdateUi()
   {
      if (ImGui::CollapsingHeader("Rigid body", ImGuiTreeNodeFlags_DefaultOpen))
      {
         float mass = mRigidBody->GetMass();
         float friction = mRigidBody->GetFriction();
         float rollingFriction = mRigidBody->GetRollingFriction();
         float restitution = mRigidBody->GetRestitution();
         bool isKinematic = mRigidBody->IsKinematic();
         glm::vec3 anisotropicFriction = mRigidBody->GetAnisotropicFriction();

         ImGui::Checkbox("Kinematic", &isKinematic);
         ImGui::SliderFloat("Mass", &mass, 0.0f, 100.0f);
         ImGui::SliderFloat("Friction", &friction, 0.00f, 10.0f);
         ImGui::SliderFloat("Rolling friction", &rollingFriction, 0.01f, 10.0f);
         ImGui::SliderFloat3("Anisotropic friction", &anisotropicFriction.x, 0.0f, 1.0f);
         ImGui::SliderFloat("Restitution", &restitution, 0.05f, 1.0f);

         mRigidBody->SetKinematic(isKinematic);
         mRigidBody->SetMass(mass);
         mRigidBody->SetFriction(friction);
         mRigidBody->SetRollingFriction(rollingFriction);
         mRigidBody->SetAnisotropicFriction(anisotropicFriction);
         mRigidBody->SetRestitution(restitution);

         ImGui::Separator();
      }
   }

   CatmullSplineInspector::CatmullSplineInspector(CCatmullSpline* catmullSpline)
   {
      mCatmullSpline = catmullSpline;
   }

   void CatmullSplineInspector::UpdateUi()
   {
      if (ImGui::CollapsingHeader("Catmull spline", ImGuiTreeNodeFlags_DefaultOpen))
      {
         bool isActive = mCatmullSpline->IsActive();
         bool isDrawingDebug = mCatmullSpline->IsDrawingDebug();
         float timePerSegment = mCatmullSpline->GetTimePerSegment();

         ImGui::Text(mCatmullSpline->GetFilename().c_str());
         ImGui::SliderFloat("Time per segment", &timePerSegment, 200.0f, 10000.0f);
         ImGui::Checkbox("Active", &isActive);
         ImGui::Checkbox("Draw debug", &isDrawingDebug);

         if (ImGui::Button("Add control point"))
         {
            mCatmullSpline->AddControlPoint();
         }

         if (ImGui::Button("Remove control point"))
         {
            mCatmullSpline->RemoveLastControlPoint();
         }

         if (ImGui::Button("Save to file"))
         {
            mCatmullSpline->SaveControlPoints();
         }

         mCatmullSpline->SetActive(isActive);
         mCatmullSpline->SetDrawDebug(isDrawingDebug);
         mCatmullSpline->SetTimePerSegment(timePerSegment);

         ImGui::Separator();
      }
   }

   PolyMeshInspector::PolyMeshInspector(CPolyMesh* polyMesh)
   {
      mPolyMesh = polyMesh;

      AddTexture("data/textures/prototype/Orange/texture_01.ktx");
      AddTexture("data/textures/prototype/Orange/texture_02.ktx");
      AddTexture("data/textures/prototype/Orange/texture_09.ktx");
      AddTexture("data/textures/prototype/Green/texture_02.ktx");
      AddTexture("data/textures/prototype/Green/texture_09.ktx");
      AddTexture("data/textures/prototype/Dark/texture_01.ktx");
      AddTexture("data/textures/prototype/Purple/texture_02.ktx");
      AddTexture("data/textures/prototype/Light/texture_02.ktx");
      AddTexture("data/textures/prototype/Light/texture_12.ktx");
   }
   
   PolyMeshInspector::~PolyMeshInspector()
   {
      for (auto& texture : mTextures)
      {
         gRenderer().GetUiOverlay()->FreeTexture(texture.identifier);
      }

      mTextures.clear();
   }

   void PolyMeshInspector::AddTexture(std::string filename)
   {
      UiTexture uiTexture;
      uiTexture.texture = Vk::gTextureLoader().LoadTexture(filename);
      uiTexture.identifier = gRenderer().GetUiOverlay()->AddImage(uiTexture.texture->GetImage());

      mTextures.push_back(uiTexture);
   }

   void PolyMeshInspector::UpdateUi()
   {
      if (ImGui::CollapsingHeader("PolyMesh", ImGuiTreeNodeFlags_DefaultOpen))
      {
         ImGui::Text("PolyMesh inspector");

         uint32_t i = 1;
         for (auto& uiTexture : mTextures)
         {
            if (ImGui::ImageButton(uiTexture.identifier, ImVec2(64, 64)))
            {
               CRenderable* renderable = mPolyMesh->GetParent()->GetComponent<CRenderable>();
               renderable->SetDiffuseTexture(0, Vk::gTextureLoader().LoadTexture(uiTexture.texture->GetPath()));
               mPolyMesh->SetTexturePath(uiTexture.texture->GetPath());
            }

            if (i % 3 != 0)
               ImGui::SameLine();

            i++;
         }

         ImGui::Separator();
      }
   }

   PlayerControllerInspector::PlayerControllerInspector(CPlayerControl* playerController)
   {
      mPlayerController = playerController;
   }

   void PlayerControllerInspector::UpdateUi()
   {
      if (ImGui::CollapsingHeader("Player controller", ImGuiTreeNodeFlags_DefaultOpen))
      {
         float maxSpeed = mPlayerController->GetMaxSpeed();
         float jumpStrength = mPlayerController->GetJumpStrength();
         float airAcceleration = mPlayerController->GetAirAcceleration();
         float groundAcceleration = mPlayerController->GetGroundAcceleration();
         float airSpeedCap = mPlayerController->GetAirSpeedCap();

         ImGui::SliderFloat("Max speed", &maxSpeed, 0.5f, 10.0f);
         ImGui::SliderFloat("Jump strength", &jumpStrength, 0.5f, 20.0f);
         ImGui::SliderFloat("Air acceleration", &airAcceleration, 0.5f, 500.0f);
         ImGui::SliderFloat("Ground acceleration", &groundAcceleration, 0.5f, 500.0f);
         ImGui::SliderFloat("Air speed cap", &airSpeedCap, 0.05f, 1.0f);
         ImGui::Text("Movement state: %u", mPlayerController->GetMovementState());
         ImGui::Text("Speed %.2f", mPlayerController->GetCurrentSpeed());
         ImGui::SliderFloat("Hand Z", &mPlayerController->handZ, -0.50f, 1.0f);
         ImGui::SliderFloat("Hand Y", &mPlayerController->handY, -1.00f, 1.0f);

         mPlayerController->SetMaxSpeed(maxSpeed);
         mPlayerController->SetJumpStrength(jumpStrength);
         mPlayerController->SetAirAcceleration(airAcceleration);
         mPlayerController->SetGroundAcceleration(groundAcceleration);
         mPlayerController->SetAirSpeedCap(airSpeedCap);

         ImGui::Separator();
      }
   }

   NoClipInspector::NoClipInspector(CNoClip* noClip)
   {
      mNoClip = noClip;
   }

   void NoClipInspector::UpdateUi()
   {
      if (ImGui::CollapsingHeader("Noclip", ImGuiTreeNodeFlags_DefaultOpen))
      {
         float speed = mNoClip->GetSpeed();

         ImGui::InputFloat("Speed", &speed);
         mNoClip->SetSpeed(speed);
      }
   }
}
