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

	class DeferredEffect : public Effect
	{
	public:
		enum Variation {
			NORMAL = 0
		};

		class UniformBufferVS : public Utopian::Vk::ShaderBuffer
		{
		public:
			virtual void UpdateMemory() {
				// Map uniform buffer and update it
				uint8_t *mapped;
				mBuffer->MapMemory(0, sizeof(data), 0, (void**)&mapped);
				memcpy(mapped, &data, sizeof(data));
				mBuffer->UnmapMemory();
			}

			virtual int GetSize() {
				return sizeof(data);
			}

			struct {
				glm::mat4 projection;
				glm::mat4 view;
			} data;
		};

		DeferredEffect();

		// Override the base class interfaces
		virtual void CreateDescriptorPool(Device* device);
		virtual void CreateVertexDescription(Device* device);
		virtual void CreatePipelineInterface(Device* device);
		virtual void CreateDescriptorSets(Device* device);
		virtual void CreatePipeline(Renderer* renderer);
		virtual void UpdateMemory(Device* device);

		DescriptorSet* mDescriptorSet0; // set = 0 in GLSL
		UniformBufferVS per_frame_vs;
	private:
	};
}
