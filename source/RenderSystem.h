#pragma once
#include "System.h"

namespace VulkanLib
{
	class VulkanApp;
}

namespace ECS
{
	// RenderSystem will need to contain low level Vulkan code but by using the wrapper classes in VulkanLib as much as possible
	// It will be RenderSystem that is responsible for how objects are stored when using different amount of CPU threads
	class RenderSystem : public System
	{
	public:
		RenderSystem(VulkanLib::VulkanApp* vulkanApp);
		virtual void Update(Entity* entity);
	private:
		// RenderSystem should not BE a VulkanApp but rather have one, since the usage of Vulkan should not be limited to only the ECS
		// VulkanApp needs to be available for HUDS, debugging etc
		VulkanLib::VulkanApp* mVulkanApp;

		// The RenderSystem should contain a list of all loaded meshes, with only one copy of each in memory
	};
}