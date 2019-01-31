#pragma once
#pragma once

#include <glm/glm.hpp>
#include "vulkan/Effect.h"
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/PipelineInterface.h"
#include "core/CommonBuffers.h"

namespace Utopian
{
	class Light;
	struct RenderingSettings;
}

namespace Utopian::Vk
{

UNIFORM_BLOCK_BEGIN(BlurSettingsBlock)
	UNIFORM_PARAM(int, blurRange)
UNIFORM_BLOCK_END()

	class BlurEffect : public Effect
	{
	public:
		BlurEffect(Device* device, RenderPass* renderPass);

		// Note: the normal image contains normals in view space
		void BindSSAOOutput(Image* ssaoImage, Sampler* sampler);
		void SetSettings(int blurRange);

		virtual void UpdateMemory();

		Utopian::Vk::Texture* noiseTexture;
		BlurSettingsBlock settingsBlock;
	private:
	};
}
