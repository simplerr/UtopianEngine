#include "core/renderer/RendererUtility.h"
#include "vulkan/handles/CommandBuffer.h"

namespace Utopian
{
	RendererUtility& gRendererUtility()
	{
		return RendererUtility::Instance();
	}

	void RendererUtility::DrawFullscreenQuad(Vk::CommandBuffer* commandBuffer)
	{
		commandBuffer->CmdDraw(3, 1, 0, 0);
	}
}