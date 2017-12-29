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

	class NormalDebugEffect : public Effect
	{
	public:
		class UniformBufferGS : public ShaderBuffer
		{
		public:
			virtual void UpdateMemory(VkDevice device);
			virtual int GetSize();

			// Public data members
			struct {
				glm::mat4 projection;
				glm::mat4 view;
			} data;
		};

		struct PushConstantBlock
		{
			glm::mat4 world;
			glm::mat4 worldInvTranspose;
		};

		NormalDebugEffect();

		// Override the base class interfaces
		virtual void CreateDescriptorPool(Device* device);
		virtual void CreateVertexDescription(Device* device);
		virtual void CreatePipelineInterface(Device* device);
		virtual void CreateDescriptorSets(Device* device);
		virtual void CreatePipeline(Renderer* renderer);
		virtual void UpdateMemory(Device* device);

		DescriptorSet* mDescriptorSet0; // set = 0 in GLSL

		UniformBufferGS per_frame_gs;
	};
}
