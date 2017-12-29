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

	/** \brief
	*
	* This effect expects the vertices to be in NDC space and simply applies a texture to them.
	* Should be used for rendering texture quads to the screen.
	**/
	class WaterEffect : public Effect
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
				float moveFactor;
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

		struct PushConstantBlock
		{
			glm::mat4 world;
			glm::vec3 color;
		};

		WaterEffect();

		// Override the base class interfaces
		virtual void CreateDescriptorPool(Device* device);
		virtual void CreateVertexDescription(Device* device);
		virtual void CreatePipelineInterface(Device* device);
		virtual void CreateDescriptorSets(Device* device);
		virtual void CreatePipeline(Renderer* renderer);
		virtual void UpdateMemory(Device* device);

		DescriptorSet* mDescriptorSet0; // set = 0 in GLSL

		UniformBufferVS per_frame_vs;
		UniformBufferPS per_frame_ps;	// Same name in terrain.frag
	};
}
