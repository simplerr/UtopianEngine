#pragma once
#include "core/components/Component.h"
#include <vector>

namespace Utopian
{
	class CCatmullSpline : public Component
	{
	public:
		CCatmullSpline(Actor* parent);
		~CCatmullSpline();

		void Update() override;
		void OnCreated() override;
		void OnDestroyed() override;
		void PostInit() override;

		LuaPlus::LuaObject GetLuaObject() override;

		// Type identification
		static uint32_t GetStaticType() {
			return Component::ComponentType::CATMULL_SPLINE;
		}

		virtual uint32_t GetType() {
			return GetStaticType();
		}
	private:
		std::vector<glm::vec3> mControlPoints;

	};
}
