#pragma once
#include "scene/SceneComponent.h"
#include "scene/Renderable.h"
#include "utility/Common.h"

namespace Scene
{
	class Actor;

	class CRenderable : public SceneComponent
	{
	public:
		CRenderable(Actor* parent);
		~CRenderable();

		void Update() override;
		void OnCreated() override;

		void SetModel(Vulkan::StaticModel* model);

		void EnableBoundingBox();
		void DisableBoundingBox();

		// Type identification
		static uint32_t GetStaticType() {
			return SceneComponent::ComponentType::STATIC_MESH;
		}

		virtual uint32_t GetType() {
			return GetStaticType();
		}

		const Vulkan::BoundingBox GetBoundingBox() const;

	private:
		SharedPtr<Renderable> mInternal;
	};
}
