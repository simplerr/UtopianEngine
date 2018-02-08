#pragma once
#include <glm/glm.hpp>
#include "scene/SceneNode.h"
#include "scene/SceneComponent.h"
#include "utility/Common.h"
#include "vulkan/VulkanInclude.h"

namespace Vk::Test
{
	class A
	{

	};
}

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

		Utopian::Vk::StaticModel* GetModel();
		void SetModel(Utopian::Vk::StaticModel* model);

		const Utopian::Vk::BoundingBox GetBoundingBox() const;

	private:
		Utopian::Vk::StaticModel* mModel;
	};
}
