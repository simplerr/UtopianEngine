#pragma once

#include <glm/glm.hpp>
#include "vulkan/handles/Effect.h"
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/PipelineInterface.h"

namespace Utopian::Vk
{
UNIFORM_BLOCK_BEGIN(ViewProjection)
	UNIFORM_PARAM(glm::mat4, projection)
	UNIFORM_PARAM(glm::mat4, view)
UNIFORM_BLOCK_END()

	class ColorEffect : public Effect
	{
	public:
		ColorEffect(Device* device, RenderPass* renderPass);

		void SetCameraData(glm::mat4 view, glm::mat4 projection);
		void UpdateMemory();
		void BindDeferredOutput(Image* deferredImage, Sampler* sampler);
	private:
		ViewProjection viewProjectionBlock;
	};
}
