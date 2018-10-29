#pragma once

#include <glm/glm.hpp>
#include "vulkan/VulkanInclude.h"
#include "vulkan/handles/Effect.h"
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/PipelineInterface.h"
#include "core/CommonBuffers.h"

namespace Utopian
{
	class Light;
	struct RenderingSettings;
}

UNIFORM_BLOCK_BEGIN(DeferredEyePos)
	UNIFORM_PARAM(glm::vec4, eyePos)
UNIFORM_BLOCK_END()

namespace Utopian::Vk
{
	class DeferredEffect : public Effect
	{
	public:
		DeferredEffect(Device* device, RenderPass* renderPass);

		void SetEyePos(glm::vec3 eyePos);
		void BindImages(Image* positionImage, Image* normalImage, Image* albedoImage, Image* ssaoImage, Sampler* sampler);
		void SetLightArray(const std::vector<Light*>& lights);
		void SetFogData(const RenderingSettings& renderingSettings);

		virtual void UpdateMemory();

		DeferredEyePos eyeBlock;
		LightUniformBuffer light_ubo;
		FogUniformBuffer fog_ubo;
	private:
	};
}
