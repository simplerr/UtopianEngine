#include "core/components/CCatmullSpline.h"
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
		float y = -1300.0f;
		mControlPoints.push_back(glm::vec3(0.0f, y, 0.0f));
		mControlPoints.push_back(glm::vec3(0.0f, y + 200, 500.0f));
		mControlPoints.push_back(glm::vec3(500.0f, y, 200.0f));
		mControlPoints.push_back(glm::vec3(500.0f, y + 100, 0.0f));

		mControlPoints.push_back(glm::vec3(1000.0f, y, 0.0f));
		mControlPoints.push_back(glm::vec3(500.0f, y + 200, 500.0f));
		mControlPoints.push_back(glm::vec3(0.0f, y, 700.0f));
		mControlPoints.push_back(glm::vec3(500.0f, y + 100, -500.0f));
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
			Im3d::DrawPoint(mControlPoints[i], 30.0f, colors[i]);
			Im3d::Mat4 im3dTransform = Im3d::Mat4(glm::translate(glm::mat4(), mControlPoints[i]));
			if (Im3d::Gizmo(std::string("SplineGizmo " + std::to_string(i)).c_str(), im3dTransform))
			{
				mControlPoints[i] = im3dTransform.getTranslation();
			}
		}

		for (float t = 0; t < 1.0f; t += 0.02f)
		{
			glm::vec3 point = glm::catmullRom(mControlPoints[0], mControlPoints[1], mControlPoints[2], mControlPoints[3], t);
			Im3d::DrawPoint(point, 20.0f, Im3d::Color_Yellow);
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
	}

	LuaPlus::LuaObject CCatmullSpline::GetLuaObject()
	{
		return LuaPlus::LuaObject();
	}
}