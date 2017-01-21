#include "RenderSystem.h"
#include "Entity.h"

namespace ECS
{
	RenderSystem::RenderSystem(VulkanLib::VulkanApp* vulkanApp)
	{
		mVulkanApp = vulkanApp;
	}

	void RenderSystem::Update(Entity* entity)
	{
		// This will not work since we want to render all identical objects after each other
		// How the RenderSystem stores the meshes internally will not be the same as in Entity

		// It does not make sense for Update() to take an Entity as RenderSystem will not do the rendering 1..n for each Entity
	}
}