#include "core/components/CCatmullSpline.h"
#include "core/components/CTransform.h"
#include "core/components/CCamera.h"
#include "core/components/Actor.h"
#include "im3d/im3d.h"
#include "utility/Timer.h"
#include <fstream>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/spline.hpp"
#include <glm/gtc/matrix_transform.hpp>

#define NEW_SEGMENT_LENGTH 5.0f

namespace Utopian
{
   CCatmullSpline::CCatmullSpline(Actor* parent, std::string filename)
      : Component(parent)
   {
      SetName("CCatmullSpline");

      mFilename = filename;
      mTimePerSegment = 1000.0f;
      mActive = false;
      mDrawDebug = true;
      LoadControlPoints(filename);
   }

   CCatmullSpline::~CCatmullSpline()
   {
   }

   void CCatmullSpline::Update()
   {
      float time = (float)gTimer().GetTime();

      if (IsDrawingDebug())
      {
         DrawDebug(time);
      }

      if (IsActive())
      {
         mTransform->SetPosition(GetPosition(time));
         mCamera->LookAt(GetPosition(time + 50));
      }
   }

   void CCatmullSpline::LoadControlPoints(std::string filename)
   {
      float x, y, z;

      std::ifstream fin(filename);
      while (fin >> x >> y >> z)
      {
         AddControlPoint(glm::vec3(x, y, z));
      }

      fin.close();
   }

   void CCatmullSpline::SaveControlPoints()
   {
      std::ofstream fout(mFilename);
      for (auto& controlPoint : mControlPoints)
      {
         fout << controlPoint.x << " " << controlPoint.y << " " << controlPoint.z << std::endl;
      }

      fout.close();
   }

   void CCatmullSpline::DrawDebug(float time)
   {
      const Im3d::Color colors[] = {
         Im3d::Color_Red,
         Im3d::Color_Green,
         Im3d::Color_Blue,
         Im3d::Color_Black,
      };

      for (uint32_t i = 0; i < mControlPoints.size(); i++)
      {
         Im3d::DrawPoint(mControlPoints[i], 30.0f, colors[i % 4]);
         Im3d::Mat4 im3dTransform = Im3d::Mat4(glm::translate(glm::mat4(), mControlPoints[i]));
         if (Im3d::Gizmo(std::string("SplineGizmo " + std::to_string(i)).c_str(), im3dTransform))
         {
            mControlPoints[i] = im3dTransform.getTranslation();
         }
      }

      for (uint32_t n = 1; n < mControlPoints.size() - 2; n++)
      {
         for (float t = 0; t < 1.0f; t += 0.02f)
         {
            glm::vec3 point = glm::catmullRom(mControlPoints[n-1], mControlPoints[n], mControlPoints[n+1], mControlPoints[n+2], t);
            Im3d::DrawPoint(point, 20.0f, Im3d::Color_Yellow);
         }
      }

      glm::vec3 position = GetPosition(time);
      Im3d::DrawPoint(position, 30.0f, Im3d::Color_Blue);
   }

   void CCatmullSpline::OnCreated()
   {
   }

   void CCatmullSpline::OnDestroyed()
   {
   }

   void CCatmullSpline::PostInit()
   {
      mTransform = GetParent()->GetComponent<CTransform>();
      mCamera = GetParent()->GetComponent<CCamera>();
   }

   LuaPlus::LuaObject CCatmullSpline::GetLuaObject()
   {
      LuaPlus::LuaObject luaObject;
      luaObject.AssignNewTable(gLuaManager().GetLuaState());

      luaObject.SetString("filename", GetFilename().c_str());
      luaObject.SetNumber("time_per_segment", GetTimePerSegment());
      luaObject.SetNumber("draw_debug", IsDrawingDebug());

      return luaObject;
   }

   void CCatmullSpline::AddControlPoint()
   {
      glm::vec3 lastControlPoint = mControlPoints.back();
      glm::vec3 lastSegmentPosition = GetPosition(GetTotalTime());
      glm::vec3 direction = glm::normalize(lastControlPoint - lastSegmentPosition);
      AddControlPoint(lastControlPoint + direction * NEW_SEGMENT_LENGTH);
   }

   void CCatmullSpline::AddControlPoint(glm::vec3 controlPoint)
   {
      mControlPoints.push_back(controlPoint);
   }
   
   void CCatmullSpline::RemoveLastControlPoint()
   {
      if (mControlPoints.size() > 4)
         mControlPoints.pop_back();
   }

   glm::vec3 CCatmullSpline::GetPosition(float time) const
   {
      float splineTime = fmod(time, GetTotalTime());
      int n = 1 + (int)((int)splineTime / mTimePerSegment); // n=1 is the first segment

      float segmentTime = fmod(splineTime, mTimePerSegment);
      glm::vec3 position = glm::catmullRom(mControlPoints[n-1], mControlPoints[n], mControlPoints[n+1], mControlPoints[n+2], segmentTime / mTimePerSegment);

      return position;
   }

   float CCatmullSpline::GetTotalTime() const
   {
      assert(mControlPoints.size() > 3);

      return mTimePerSegment * (mControlPoints.size() - 3);
   }

   float CCatmullSpline::GetTimePerSegment() const
   {
      return mTimePerSegment;
   }

   bool CCatmullSpline::IsActive() const
   {
      return mActive;
   }

   bool CCatmullSpline::IsDrawingDebug() const
   {
      return mDrawDebug;
   }

   std::string CCatmullSpline::GetFilename() const
   {
      return mFilename;
   }

   void CCatmullSpline::SetActive(bool active)
   {
      mActive = active;
   }

   void CCatmullSpline::SetDrawDebug(bool drawDebug)
   {
      mDrawDebug = drawDebug;
   }

   void CCatmullSpline::SetTimePerSegment(float timePerSegment)
   {
      mTimePerSegment = timePerSegment;
   }
}