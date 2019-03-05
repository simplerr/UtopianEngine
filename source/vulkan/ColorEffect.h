#pragma once

#include <glm/glm.hpp>
#include "vulkan/Effect.h"
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/PipelineInterface.h"

namespace Utopian::Vk
{
	class ColorEffect : public Effect
	{
	public:
		ColorEffect(Device* device, RenderPass* renderPass);

		void BindDeferredOutput(Image* deferredImage, Sampler* sampler);
	private:
	};
}
