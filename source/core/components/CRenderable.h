#pragma once
#include "core/components/Component.h"
#include "core/renderer/Renderable.h"
#include "utility/Common.h"
#include "core/LuaManager.h"

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
		void OnDestroyed() override;
		void PostInit() override;

		LuaPlus::LuaObject GetLuaObject() override;

		void LoadModel(std::string path);
		void SetModel(Utopian::Vk::StaticModel* model);
		void SetColor(glm::vec4 color);
		void SetMaterial(Utopian::Vk::Mat material);
		void SetRenderFlags(uint32_t renderFlags);
		void AppendRenderFlags(uint32_t renderFlags);
		uint32_t GetRenderFlags() const;
		const bool HasRenderFlags(uint32_t renderFlags) const;

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
		const std::string GetPath() const;

	private:
		SharedPtr<Renderable> mInternal;
		std::string mPath;
	};
}
