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

	class Renderable : public SceneNode
	{
	public:
		Renderable();
		~Renderable();

		void Initialize();

		static SharedPtr<Renderable> Create();

		void SetModel(Utopian::Vk::StaticModel* model);
		void SetColor(glm::vec4 color);
		void SetMaterial(Utopian::Vk::Mat material);

		Utopian::Vk::StaticModel* GetModel();
		const BoundingBox GetBoundingBox() const;
		const glm::vec4 GetColor() const;
		const Utopian::Vk::Mat GetMaterial() const;

	private:
		Utopian::Vk::StaticModel* mModel;
		Utopian::Vk::Mat mMaterial;
		glm::vec4 mColor;
	};
}
