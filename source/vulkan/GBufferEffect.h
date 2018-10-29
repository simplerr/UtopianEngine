#pragma once

#include <glm/glm.hpp>
#include "vulkan/handles/Effect.h"
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/PipelineInterface.h"

namespace Utopian::Vk
{
	class Renderer;
	class DescriptorSetLayout;
	class DescriptorPool;
	class DescriptorSet;
	class Pipeline2;
	class ComputePipeline;
	class PipelineLayout;
	class VertexDescription;
	class Shader;
	class Texture;

UNIFORM_BLOCK_BEGIN(GBufferViewProjection)
	UNIFORM_PARAM(glm::mat4, projection)
	UNIFORM_PARAM(glm::mat4, view)
UNIFORM_BLOCK_END()

	class GBufferEffect : public Effect
	{
	public:
		GBufferEffect(Device* device, RenderPass* renderPass);

		void SetCameraData(glm::mat4 view, glm::mat4 projection);
		void UpdateMemory();
	private:
		GBufferViewProjection viewProjectionBlock;
	};
}
