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
			virtual void UpdateMemory(VkDevice device)
			{
				// Map uniform buffer and update it
				uint8_t *mapped;
				mBuffer->MapMemory(0, sizeof(data), 0, (void**)&mapped);
				memcpy(mapped, &data, sizeof(data));
				mBuffer->UnmapMemory();
			}

			virtual int GetSize()
			{
				return sizeof(data);
			}

			struct {
				glm::mat4 projection;
				glm::mat4 view;
				glm::vec3 eyePos;
			} data;
		};

		class UniformBufferPS : public Vulkan::ShaderBuffer
		{
		public:
			virtual void UpdateMemory(VkDevice device)
			{
				// Map uniform buffer and update it
				uint8_t *mapped;
				mBuffer->MapMemory(0, sizeof(data), 0, (void**)&mapped);
				memcpy(mapped, &data, sizeof(data));
				mBuffer->UnmapMemory();
			}

			virtual int GetSize()
			{
				return sizeof(data);
			}

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

		TerrainEffect();

		/* Member variables */
		UniformBufferVS uniformBufferVS;
		UniformBufferPS uniformBufferPS;
		DescriptorSet* mDescriptorSet;
	};
}
