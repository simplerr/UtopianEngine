#pragma once
#include <glm/glm.hpp>
#include "core/SceneNode.h"
#include "core/components/Component.h"
#include "utility/Common.h"
#include "vulkan/VulkanInclude.h"
#include "vulkan/PhongEffect.h"

namespace Utopian
{
	class Actor;

	enum RenderFlags
	{
		RENDER_FLAG_DEFERRED = 1 << 0,
		RENDER_FLAG_DEBUG = 1 << 1,
		RENDER_FLAG_NORMAL_DEBUG = 1 << 2
	};

	class Renderable : public SceneNode
	{
	public:
		Renderable();
		~Renderable();

		void Initialize();
		void OnDestroyed();

		static SharedPtr<Renderable> Create();

		void LoadModel(std::string path);

		void SetModel(Utopian::Vk::StaticModel* model);
		void SetColor(glm::vec4 color);
		void SetMaterial(Utopian::Vk::Mat material);
		void SetVisible(bool visible);
		void SetRenderFlags(uint32_t renderFlags);

		Utopian::Vk::StaticModel* GetModel();
		const BoundingBox GetBoundingBox() const;
		const glm::vec4 GetColor() const;
		const Utopian::Vk::Mat GetMaterial() const;
		const bool IsVisible() const;
		const uint32_t GetRenderFlags() const;

		const bool HasRenderFlags(uint32_t renderFlags) const;

	private:
		Utopian::Vk::StaticModel* mModel;
		Utopian::Vk::Mat mMaterial;
		glm::vec4 mColor;
		uint32_t mRenderFlags;
		bool mVisible;
	};
}
