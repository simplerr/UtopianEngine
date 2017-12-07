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
	class ComputePipeline;
	class PipelineLayout;
	class VertexDescription;
	class Shader;
	class Texture;

	struct BasicVertex
	{
		glm::vec4 position;
		glm::vec4 normal;
	};

	/** \brief Most basic effect
	*
	* Simply transforms each vertex and sets a pixel color
	**/
	class OffscreenEffect : public Effect
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

		OffscreenEffect();

		// Override the base class interfaces
		virtual void CreateDescriptorPool(Device* device);
		virtual void CreateVertexDescription(Device* device);
		virtual void CreatePipelineInterface(Device* device);
		virtual void CreateDescriptorSets(Device* device);
		virtual void CreatePipeline(Renderer* renderer);
		virtual void UpdateMemory(Device* device);

		UniformBufferVS per_frame_vs;	// Same name in terrain.vert
		DescriptorSet* mDescriptorSet0; // set = 0 in GLSL
	private:
	};
}
