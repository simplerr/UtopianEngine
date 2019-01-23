#pragma once

#include "utility\Module.h"
#include "vulkan\VulkanInclude.h"

namespace Utopian
{
	class RendererUtility : public Module<RendererUtility>
	{
	public:
		void DrawFullscreenQuad(Vk::CommandBuffer* commandBuffer);
		//void DrawMesh(...);
	private:

	};

	RendererUtility& gRendererUtility();
}
