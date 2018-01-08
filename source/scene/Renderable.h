#pragma once
#include <glm/glm.hpp>
#include "scene/SceneNode.h"
#include "scene/SceneComponent.h"
#include "Common.h"

namespace Vulkan
{
	class StaticModel;
}

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

	private:
		Vulkan::StaticModel* mModel;
	};
}
