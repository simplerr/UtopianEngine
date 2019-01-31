#pragma once

#include <glm/glm.hpp>
#include "vulkan/Effect.h"
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/PipelineInterface.h"

namespace Utopian::Vk
{
	class SkyboxEffect : public Effect
	{

	public:

UNIFORM_BLOCK_BEGIN(ViewProjection)
	UNIFORM_PARAM(glm::mat4, projection)
	UNIFORM_PARAM(glm::mat4, view)
	UNIFORM_PARAM(glm::mat4, world)
UNIFORM_BLOCK_END()

		SkyboxEffect(Device* device, RenderPass* renderPass);

		void SetCameraData(glm::mat4 view, glm::mat4 projection);
		void UpdateMemory();
	private:
		ViewProjection viewProjectionBlock;
	};
}
