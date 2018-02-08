#pragma once
#include "scene/Component.h"
#include "scene/Renderable.h"
#include "utility/Common.h"

namespace Utopian
{
	class Actor;

	class CRenderable : public Component
	{
	public:
		CRenderable(Actor* parent);
		~CRenderable();

		void Update() override;
		void OnCreated() override;

		void SetModel(Utopian::Vk::StaticModel* model);

		void EnableBoundingBox();
		void DisableBoundingBox();

		// Type identification
		static uint32_t GetStaticType() {
			return Component::ComponentType::STATIC_MESH;
		}

		virtual uint32_t GetType() {
			return GetStaticType();
		}

		const BoundingBox GetBoundingBox() const;

	private:
		SharedPtr<Renderable> mInternal;
	};
}
