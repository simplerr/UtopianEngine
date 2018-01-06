#pragma once
#include "scene/SceneComponent.h"
#include "scene/Renderable.h"
#include "Common.h"

namespace Scene
{
	class SceneEntity;

	class CRenderable : public SceneComponent
	{
	public:
		CRenderable(SceneEntity* parent);
		~CRenderable();

		void Update() override;
		void OnCreated() override;

		void SetModel(Vulkan::StaticModel* model);

		// Type identification
		static uint32_t GetStaticType() {
			return SceneComponent::ComponentType::STATIC_MESH;
		}

		virtual uint32_t GetType() {
			return GetStaticType();
		}

	private:
		SharedPtr<Renderable> mInternal;
	};
}
