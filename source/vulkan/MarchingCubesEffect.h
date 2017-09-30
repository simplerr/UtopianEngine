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


	/** \brief Most basic effect
	*
	* Simply transforms each vertex and sets a pixel color
	**/
	class MarchingCubesEffect : public Effect
	{
	public:
		class UniformBuffer : public Vulkan::ShaderBuffer
		{
		public:
			virtual void UpdateMemory(VkDevice device);
			virtual int GetSize();

			// Public data members
			struct {
				glm::mat4 projection;
				glm::mat4 view;
				glm::vec4 offsets[8];
				glm::vec4 color;
				float voxelSize;
				float time;
			} data;
		};

		MarchingCubesEffect();

		// Override the base class interfaces
		virtual void CreateDescriptorPool(Device* device);
		virtual void CreateVertexDescription(Device* device);
		virtual void CreatePipelineInterface(Device* device);
		virtual void CreateDescriptorSets(Device* device);
		virtual void CreatePipeline(Renderer* renderer);

		/* Updates the memory for the effects descriptors
		*/
		virtual void UpdateMemory(Device* device);

		// TODO: This should be handled in the baseclass.
		// Multiple compute effects will use the same code
		ComputePipeline* GetComputePipeline();

		/* Shader descriptors */
		UniformBuffer ubo;
		Vulkan::Texture* edgeTableTex;
		Vulkan::Texture* triangleTableTex;
		DescriptorSet* mDescriptorSet0; // set = 0 in GLSL

		const uint32_t NUM_MAX_STORAGE_BUFFERS = 200;
	private:
		Vulkan::ComputePipeline* mComputePipeline;
	};
}
