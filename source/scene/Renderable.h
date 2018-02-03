#pragma once
#include <glm/glm.hpp>
#include "scene/SceneNode.h"
#include "scene/SceneComponent.h"
#include "utility/Common.h"
#include "vulkan/VulkanInclude.h"

namespace Scene
{
	class SceneEntity;

	class Renderable : public SceneNode
	{
	public:
		Renderable();
		~Renderable();

		void Initialize();

		static SharedPtr<Renderable> Create();

		Vulkan::StaticModel* GetModel();
		void SetModel(Vulkan::StaticModel* model);

		const Vulkan::BoundingBox GetBoundingBox() const;

	private:
		Vulkan::StaticModel* mModel;
	};
}
