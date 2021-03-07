#pragma once
#include "core/Transform.h"
#include "core/LightData.h"
#include "imgui/imgui.h"
#include "vulkan/Texture.h"
#include <string>
#include <vector>

namespace Utopian
{
   class CTransform;
   class CRenderable;
   class CLight;
   class CRigidBody;
   class CCatmullSpline;
   class CPolyMesh;
   class CPlayerControl;
   class CNoClip;

   class ComponentInspector
   {
   public:
      ComponentInspector();
      virtual ~ComponentInspector();

      virtual void UpdateUi() = 0;
   private:
   };

   class TransformInspector : public ComponentInspector
   {
   public:
      TransformInspector(CTransform* transform);

      virtual void UpdateUi() override;
   private:
      CTransform* mComponent;
      Transform mTransform;
   };

   class RenderableInspector : public ComponentInspector
   {
   public:
      struct TextureInfo
      {
         TextureInfo(ImTextureID id, std::string _path) {
            textureId = id;
            path = _path;
         }

         ImTextureID textureId;
         std::string path;
      };

      RenderableInspector(CRenderable* renderable);
      ~RenderableInspector();

      virtual void UpdateUi() override;
   private:
      CRenderable* mRenderable;
      float mTextureTiling;
      bool mDeferred;
      bool mColor;
      bool mBoundingBox;
      bool mDebugNormals;
      bool mWireframe;
      bool mCastShadow;
      bool mVisible;

      // For ImGui debug drawing
      std::vector<TextureInfo> textureInfos;
   };

   class LightInspector : public ComponentInspector
   {
   public:
      LightInspector(CLight* light);

      virtual void UpdateUi() override;
   private:
      CLight* mLight;
      Utopian::LightData mLightData;
      int mType;
   };

   class RigidBodyInspector : public ComponentInspector
   {
   public:
      RigidBodyInspector(CRigidBody* rigidBody);

      virtual void UpdateUi() override;
   private:
      CRigidBody* mRigidBody;
   };

   class CatmullSplineInspector : public ComponentInspector
   {
   public:
      CatmullSplineInspector(CCatmullSpline* catmullSpline);

      virtual void UpdateUi() override;
   private:
      CCatmullSpline* mCatmullSpline;
   };

   class PolyMeshInspector : public ComponentInspector
   {
      struct UiTexture
      {
         ImTextureID identifier;
         SharedPtr<Vk::Texture> texture;
      };
   public:
      PolyMeshInspector(CPolyMesh* polyMesh);

      virtual void UpdateUi() override;

      void AddTexture(std::string filename);
   private:
      CPolyMesh* mPolyMesh;
      std::vector<UiTexture> mTextures;
   };

   class PlayerControllerInspector : public ComponentInspector
   {
   public:
      PlayerControllerInspector(CPlayerControl* playerController);

      virtual void UpdateUi() override;
   private:
      CPlayerControl* mPlayerController;
   };

   class NoClipInspector : public ComponentInspector
   {
   public:
      NoClipInspector(CNoClip* noClip);

      virtual void UpdateUi() override;
   private:
      CNoClip* mNoClip;
   };
}
