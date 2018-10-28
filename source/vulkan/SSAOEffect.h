#pragma once

#include <glm/glm.hpp>
#include "vulkan/EffectLegacy.h"
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/Effect.h"
#include "core/CommonBuffers.h"

namespace Utopian
{
	class Light;
	struct RenderingSettings;
}

namespace Utopian::Vk
{
	class Renderer;
	class DescriptorSetLayout;
	class DescriptorPool;
	class DescriptorSet;
	class Effect;
	class ComputePipeline;
	class PipelineLayout;
	class VertexDescription;
	class Shader;
	class Texture;

UNIFORM_BLOCK_BEGIN(SSAOCameraBlock)
	UNIFORM_PARAM(glm::mat4, projection)
	UNIFORM_PARAM(glm::mat4, view)
	UNIFORM_PARAM(glm::vec4, eyePos)
	UNIFORM_PARAM(glm::vec4, samples[64]) // Todo: Move to it's own block so the rest can be reused
UNIFORM_BLOCK_END()

UNIFORM_BLOCK_BEGIN(SSAOSettingsBlock)
	UNIFORM_PARAM(float, radius)
	UNIFORM_PARAM(float, bias)
UNIFORM_BLOCK_END()

	class SSAOEffect : public Effect
	{
	public:
		enum Variation {
			NORMAL = 0
		};

		SSAOEffect(Device* device, RenderPass* renderPass);

		void SetCameraData(glm::mat4 view, glm::mat4 projection, glm::vec4 eyePos);
		void SetSettings(float radius, float bias);

		// Note: the normal image contains normals in view space
		void BindGBuffer(Image* positionImage, Image* normalViewImage, Image* albedoImage, Sampler* sampler);

		virtual void UpdateMemory();

		Utopian::Vk::Texture* noiseTexture;
		RenderPass* mRenderPass;

		SSAOCameraBlock cameraBlock;
		SSAOSettingsBlock settingsBlock;
	private:
	};
}
