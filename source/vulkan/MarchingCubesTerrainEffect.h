#pragma once

#include <glm/glm.hpp>
#include "vulkan/EffectLegacy.h"
#include "vulkan/Texture.h"
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/PipelineInterface.h"

namespace Utopian::Vk
{
	class VulkanApp;
	class DescriptorSetLayout;
	class DescriptorPool;
	class DescriptorSet;
	class Pipeline2;
	class PipelineLayout;
	class VertexDescription;
	class Shader;

	struct PushConstantBasicBlock {
		glm::mat4 world;
		glm::vec3 color;
	};

	class MarchingCubesTerrainEffect : public EffectLegacy
	{
	public:
		enum PipelineType2
		{
			SOLID = 0,
			WIREFRAME 
		};

		// Override the base class interfaces
		virtual void CreateDescriptorPool(Device* device);
		virtual void CreateVertexDescription(Device* device);
		virtual void CreatePipelineInterface(Device* device);
		virtual void CreateDescriptorSets(Device* device);
		virtual void CreatePipeline(Device* device, RenderPass* renderPass);

		/* Updates the memory for the effects descriptors
		*/
		virtual void UpdateMemory();

		MarchingCubesTerrainEffect();
	};
}
