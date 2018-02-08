#pragma once

#include <glm/glm.hpp>
#include "vulkan/Effect.h"
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/PipelineInterface.h"
#include "LightData.h"

namespace Utopian::Vk
{
	class Renderer;
	class DescriptorSetLayout;
	class DescriptorPool;
	class DescriptorSet;
	class Pipeline2;
	class PipelineLayout;
	class VertexDescription;
	class Shader;

	/** \brief Most basic effect
	*
	* Simply transforms each vertex and sets a pixel color
	**/
	class PhongEffect : public Effect
	{
	public:
		enum PipelineType2
		{
			BASIC = 0,
			WIREFRAME,
			TEST,
			DEBUG
		};

		// Override the base class interfaces
		virtual void CreateDescriptorPool(Device* device);
		virtual void CreateVertexDescription(Device* device);
		virtual void CreatePipelineInterface(Device* device);
		virtual void CreateDescriptorSets(Device* device);
		virtual void CreatePipeline(Renderer* renderer);

		/* Updates the memory for the effects descriptors
		*/
		virtual void UpdateMemory(Device* device);

		PhongEffect();

		DescriptorSet* mCommonDescriptorSet;

		// TODO: Should this really be here?
		const uint32_t					MAX_NUM_TEXTURES = 64;
	};
}
