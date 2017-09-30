#pragma once

#include <glm/glm.hpp>
#include "vulkan/Effect.h"
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/PipelineInterface.h"

namespace Vulkan
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
		class UniformBufferVS : public Vulkan::ShaderBuffer
		{
		public:
			virtual void UpdateMemory(VkDevice device);
			virtual int GetSize();

			struct {
				glm::mat4 projection;
				glm::mat4 view;
				glm::vec3 eyePos;
			} data;
		};

		class UniformBufferPS : public Vulkan::ShaderBuffer
		{
		public:
			virtual void UpdateMemory(VkDevice device);
			virtual int GetSize();

			struct {
				glm::vec3 eyePos;
				float padding;
				float fogStart;
				float fogDistance;
			} data;
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

		/* Shader descriptors */
		UniformBufferVS per_frame_vs;	// Same name in terrain.vert
		UniformBufferPS per_frame_ps;	// Same name in terrain.frag
		DescriptorSet* mDescriptorSet0; // set = 0 in GLSL
	};
}
