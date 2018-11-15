#pragma once

#include <glm/glm.hpp>
#include "vulkan/handles/Effect.h"
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/PipelineInterface.h"

namespace Utopian::Vk
{
	// Todo: Place in common header
UNIFORM_BLOCK_BEGIN(ViewProjection1)
	UNIFORM_PARAM(glm::mat4, projection)
	UNIFORM_PARAM(glm::mat4, view)
UNIFORM_BLOCK_END()

	class NormalDebugEffect : public Effect
	{
	public:
		NormalDebugEffect(Device* device, RenderPass* renderPass);

		// Override the base class interfaces
		void UpdateMemory();
		void SetCameraData(glm::mat4 view, glm::mat4 projection);

		ViewProjection1 viewProjectionBlock;
	};
}
