#pragma once
#include "core/components/Component.h"
#include <vector>
#include <string>

namespace Utopian
{
	class CTransform;
	class CCamera;

	class CCatmullSpline : public Component
	{
	public:
		CCatmullSpline(Actor* parent, std::string filename);
		~CCatmullSpline();

		void Update() override;
		void OnCreated() override;
		void OnDestroyed() override;
		void PostInit() override;

		//! Adds a control point continuing the last segment
		void AddControlPoint();

		//! Adds a control point at a fixed position
		void AddControlPoint(glm::vec3 controlPoint);

		//! Removes the last control point
		void RemoveLastControlPoint();

		void SaveControlPoints();

		glm::vec3 GetPosition(float time) const;
		float GetTimePerSegment() const;
		bool IsActive() const;
		bool IsDrawingDebug() const;
		std::string GetFilename() const;

		void SetActive(bool active);
		void SetDrawDebug(bool drawDebug);
		void SetTimePerSegment(float timePerSegment);

		LuaPlus::LuaObject GetLuaObject() override;

		// Type identification
		static uint32_t GetStaticType() {
			return Component::ComponentType::CATMULL_SPLINE;
		}

		virtual uint32_t GetType() {
			return GetStaticType();
		}

		float GetTotalTime() const;

	private:
		void DrawDebug(float time);
		void LoadControlPoints(std::string filename);
	private:
		std::vector<glm::vec3> mControlPoints;
		std::string mFilename;
		float mTimePerSegment; // Milliseconds
		bool mActive;
		bool mDrawDebug;
		CTransform* mTransform;
		CCamera* mCamera;
	};
}
