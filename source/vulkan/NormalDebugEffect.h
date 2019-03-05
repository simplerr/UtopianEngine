#pragma once

#include <glm/glm.hpp>
#include "vulkan/Effect.h"
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/PipelineInterface.h"

namespace Utopian::Vk
{
	class NormalDebugEffect : public Effect
	{
	public:
		NormalDebugEffect(Device* device, RenderPass* renderPass);
	};
}
