#include "ComponentInspector.h"
#include "core/renderer/ImGuiRenderer.h"
#include "vulkan/Debug.h"
#include "core/components/CTransform.h"
#include "core/components/CLight.h"
#include "core/components/CRenderable.h"
#include "core/components/CRigidBody.h"
#include "core/components/CCatmullSpline.h"
#include "core/components/CPolyMesh.h"
#include "core/components/CPlayerControl.h"
#include "core/components/CNoClip.h"
#include "core/components/Actor.h"
#include "core/Log.h"
#include <glm/gtc/quaternion.hpp>
#include "vulkan/Texture.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/handles/Image.h"
#include "core/renderer/Renderer.h"
#include <algorithm>

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
      }
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

      // Todo: MODEL UPDATE
      //std::vector<Primitive*>& meshes = renderable->GetInternal()->GetModel()->mMeshes;
      std::vector<Vk::Texture*> allTextures;

      // for (auto& mesh : meshes)
      // {
      //    std::vector<Vk::Texture*> textures = mesh->GetTextures();
      //    allTextures.insert(std::end(allTextures), std::begin(textures), std::end(textures));
      // }

      std::sort(allTextures.begin(), allTextures.end(), [](Vk::Texture* textureA, Vk::Texture* textureB) {
         return textureA->GetPath() < textureB->GetPath();
      });

      auto iter = std::unique(allTextures.begin(), allTextures.end(), [](Vk::Texture* textureA, Vk::Texture* textureB) {
         return textureA->GetPath() == textureB->GetPath();
      });

      allTextures.erase(iter, allTextures.end());

      for (auto& texture : allTextures)
      {
         textureInfos.push_back(TextureInfo(gRenderer().GetUiOverlay()->AddImage(texture->GetImage()), texture->GetPath()));
      }
   }

   RenderableInspector::~RenderableInspector()
   {
      for (auto& textureInfo : textureInfos)
      {
         gRenderer().GetUiOverlay()->FreeTexture(textureInfo.textureId);
      }

      textureInfos.clear();
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

         if (ImGui::CollapsingHeader("Textures"))
         {
            for (auto& textureInfo : textureInfos)
            {
               ImGui::Text(textureInfo.path.c_str());
               if (ImGui::ImageButton(textureInfo.textureId, ImVec2(64, 64)))
               {
                  // Nothing
               }
            }
         }
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

         ImGui::ColorEdit3("Color", &mLightData.lightColor.ambient.x);
         mLightData.lightColor.diffuse = mLightData.lightColor.ambient;
         mLightData.lightColor.specular = mLightData.lightColor.ambient;

         mLight->SetIntensity(mLightData.intensity);
         mLight->SetDirection(mLightData.direction);
         mLight->SetLightColor(mLightData.lightColor);
         mLight->SetRange(mLightData.range);
         mLight->SetSpot(mLightData.spot);
         mLight->SetAttenuation(mLightData.att);
         mLight->SetType((Utopian::LightType)mType);
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
         float airAccelerate = mPlayerController->GetAirAccelerate();
         float airSpeedCap = mPlayerController->GetAirSpeedCap();

         ImGui::SliderFloat("Max speed", &maxSpeed, 0.5f, 10.0f);
         ImGui::SliderFloat("Jump strength", &jumpStrength, 0.5f, 20.0f);
         ImGui::SliderFloat("Air accelerate", &airAccelerate, 0.005f, 0.3f);
         ImGui::SliderFloat("Air speed cap", &airSpeedCap, 0.05f, 1.0f);
         ImGui::Text("Movement state: %u", mPlayerController->GetMovementState());
         ImGui::Text("Speed %.2f", mPlayerController->GetCurrentSpeed());
         ImGui::SliderFloat("Hand Z", &mPlayerController->handZ, -0.50f, 1.0f);
         ImGui::SliderFloat("Hand Y", &mPlayerController->handY, -1.00f, 1.0f);

         mPlayerController->SetMaxSpeed(maxSpeed);
         mPlayerController->SetJumpStrength(jumpStrength);
         mPlayerController->SetAirAccelerate(airAccelerate);
         mPlayerController->SetAirSpeedCap(airSpeedCap);
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
