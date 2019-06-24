#pragma once
#include "core/components/Component.h"
#include <vector>

namespace Utopian
{
	class CTransform;
	class CCamera;

	class CCatmullSpline : public Component
	{
	public:
		CCatmullSpline(Actor* parent);
		~CCatmullSpline();

		void Update() override;
		void OnCreated() override;
		void OnDestroyed() override;
		void PostInit() override;

		void AddControlPoint(glm::vec3 controlPoint);

		glm::vec3 GetPosition(float time) const;
		float GetTimePerSegment() const;
		bool IsActive() const;

		void SetActive(bool active);
		void SetTimePerSegment(float timePerSegment);

		LuaPlus::LuaObject GetLuaObject() override;

		// Type identification
		static uint32_t GetStaticType() {
			return Component::ComponentType::CATMULL_SPLINE;
		}

		virtual uint32_t GetType() {
			return GetStaticType();
		}

	private:
		float GetTotalTime() const;

	private:
		std::vector<glm::vec3> mControlPoints;
		float mTimePerSegment; // Seconds
		bool mActive;
		CTransform* mTransform;
		CCamera* mCamera;
	};
}
