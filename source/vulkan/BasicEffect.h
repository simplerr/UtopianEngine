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
	class BasicEffect
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

		BasicEffect(Renderer* renderer);

		/* Member variables */
		UniformBufferVS uniformBufferVS;
		UniformBufferPS uniformBufferPS;

		DescriptorPool* mDescriptorPool;
		DescriptorSet* mDescriptorSet;
		Pipeline2* mBasicPipeline;
		VertexDescription* mVertexDescription;
		Shader* mShader;
		PipelineInterface mPipelineInterface;
	};
}
