#include "core/components/CCatmullSpline.h"
#include "core/components/CTransform.h"
#include "core/components/CCamera.h"
#include "core/components/Actor.h"
#include "im3d/im3d.h"
#include "utility/Timer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/spline.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace Utopian
{
	CCatmullSpline::CCatmullSpline(Actor* parent)
		: Component(parent)
	{
		mTimePerSegment = 1000.0f;
		mActive = false;
	}

	CCatmullSpline::~CCatmullSpline()
	{
	}

	void CCatmullSpline::Update()
	{
		float time = gTimer().GetTime();

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

		if (IsActive())
		{
			glm::vec3 cameraTarget = GetPosition(time + 50);
			mTransform->SetPosition(position);
			mCamera->LookAt(cameraTarget);
		}
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
		return LuaPlus::LuaObject();
	}

	void CCatmullSpline::AddControlPoint(glm::vec3 controlPoint)
	{
		mControlPoints.push_back(controlPoint);
	}

	glm::vec3 CCatmullSpline::GetPosition(float time) const
	{
		float splineTime = fmod(time, GetTotalTime());
		int n = 1 + ((int)splineTime / mTimePerSegment); // n=1 is the first segment

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

	void CCatmullSpline::SetActive(bool active)
	{
		mActive = active;
	}

	void CCatmullSpline::SetTimePerSegment(float timePerSegment)
	{
		mTimePerSegment = timePerSegment;
	}
}