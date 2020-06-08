#pragma once
#include <glm/glm.hpp>
#include "core/components/Component.h"
#include "vulkan/VulkanPrerequisites.h"
#include "utility/Common.h"

namespace Utopian
{
	class Actor;
	class CCamera;
	class CTransform;
	class CLight;
	class CRenderable;

	/*
	 * Sets the CRenderables color to match the lights color.
	 */
	class CBloomLight : public Component
	{
	public:
		CBloomLight(Actor* parent);
		~CBloomLight();

		void Update() override;
		void OnCreated() override;
		void OnDestroyed() override;
		void PostInit() override;

		LuaPlus::LuaObject GetLuaObject() override;

		// Type identification
		static uint32_t GetStaticType() {
			return Component::ComponentType::BLOOM_LIGHT;
		}

		virtual uint32_t GetType() {
			return GetStaticType();
		}

	private:
		CTransform* mTransform;
		CRenderable* mRenderable;
		CLight* mLight;
	};
}
