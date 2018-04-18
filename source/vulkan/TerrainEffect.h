#pragma once

#include <glm/glm.hpp>
#include "vulkan/Effect.h"
#include "vulkan/handles/Texture.h"
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
	class PipelineLayout;
	class VertexDescription;
	class Shader;

	struct BasicVertex
	{
		glm::vec4 position;
		glm::vec4 normal;
	};

	struct PushConstantBasicBlock {
		glm::mat4 world;
		glm::vec3 color;
	};

	/** \brief Most basic effect
	*
	* Simply transforms each vertex and sets a pixel color
	**/
	class TerrainEffect : public Effect
	{
	public:
		enum PipelineType2
		{
			SOLID = 0,
			WIREFRAME 
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

		TerrainEffect();
	};
}
