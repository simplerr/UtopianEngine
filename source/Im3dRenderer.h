#pragma once
#include "vulkan/VulkanInclude.h"
#include "glm/glm.hpp"

namespace Utopian
{
	class Im3dRenderer
	{
	public:
		Im3dRenderer(glm::vec2 viewportSize);
		~Im3dRenderer();

		void NewFrame();
		void EndFrame();

	private:
		Vk::CommandBuffer* mCommandBuffer;
		Vk::Effect* mEffect;
		glm::vec2 mViewportSize;
	};
}
