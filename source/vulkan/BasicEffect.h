#pragma once

#include <glm/glm.hpp>
#include "vulkan/Effect.h"
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"

namespace Vulkan
{
	struct BasicVertex
	{
		glm::vec3 position;
	};

	/** \brief Most basic effect
	*
	* Simply transforms each vertex and sets a pixel color
	**/
	class BasicEffect : public Effect
	{
	public:
		class UniformBuffer : public Vulkan::ShaderBuffer
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
			} data;
		};

		BasicEffect(Renderer* renderer);

		/* Member variables */
		UniformBuffer uniformBuffer;
	};
}
