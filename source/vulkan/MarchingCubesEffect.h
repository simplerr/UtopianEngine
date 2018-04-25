#pragma once

#include <glm/glm.hpp>
#include "vulkan/Effect.h"
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


	/** \brief Most basic effect
	*
	* Simply transforms each vertex and sets a pixel color
	**/
	class MarchingCubesEffect : public Effect
	{
	public:
		class UniformBuffer : public Utopian::Vk::ShaderBuffer
		{
		public:
			virtual void UpdateMemory();
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

		class CounterSSBO : public Utopian::Vk::ShaderBuffer
		{
		public:
			virtual void UpdateMemory()
			{
				// Map vertex counter
				uint8_t *mapped;
				uint32_t offset = 0;
				uint32_t size = sizeof(numVertices);
				mBuffer->MapMemory(offset, size, 0, (void**)&mapped);
				memcpy(mapped, &numVertices, size);
				mBuffer->UnmapMemory();
			}

			virtual int GetSize()
			{
				return sizeof(numVertices);
			}

			uint32_t numVertices;
		};

		MarchingCubesEffect();

		// Override the base class interfaces
		virtual void CreateDescriptorPool(Device* device);
		virtual void CreateVertexDescription(Device* device);
		virtual void CreatePipelineInterface(Device* device);
		virtual void CreateDescriptorSets(Device* device);
		virtual void CreatePipeline(Renderer* renderer);
		virtual void UpdateMemory();

		VkDescriptorSet GetDescriptorSet0();
		VkDescriptorSet GetDescriptorSet1();

		// TODO: This should be handled in the baseclass.
		// Multiple compute effects will use the same code
		ComputePipeline* GetComputePipeline();

		/* Shader descriptors */
		UniformBuffer ubo;
		Utopian::Vk::Texture* edgeTableTex;
		Utopian::Vk::Texture* triangleTableTex;
		Utopian::Vk::Texture* texture3d;
		DescriptorSet* mDescriptorSet0; // set = 0 in GLSL
		DescriptorSet* mDescriptorSet1; // set = 1 in GLSL

		CounterSSBO mCounterSSBO;

		const uint32_t NUM_MAX_STORAGE_BUFFERS = 40;
	private:
		Utopian::Vk::ComputePipeline* mComputePipeline;
	};
}
