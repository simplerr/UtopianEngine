#pragma once
#include <glm/glm.hpp>
#include "scene/SceneComponent.h"
#include "Common.h"

namespace Vulkan
{
	class StaticModel;
}

namespace Scene
{
	class SceneEntity;

	class Renderable
	{
	public:
		Renderable();
		~Renderable();

		void Initialize();

		static SharedPtr<Renderable> Create();

		Vulkan::StaticModel* GetModel();
		void SetModel(Vulkan::StaticModel* model);

		glm::vec3 pos;
		glm::vec3 scale;

	private:
		Vulkan::StaticModel* mModel;
	};
}
