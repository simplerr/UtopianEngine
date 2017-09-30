#pragma once

#include <vulkan\vulkan.h>
#include "vulkan/PipelineInterface.h"

namespace Vulkan
{
	class Renderer;
	class Device;
	class DescriptorPool;
	class VertexDescription;
	class Pipeline2;

	/* 
	 * Base class for all effects.
	 *
	 * \note The naming of the descriptors in C++ and GLSL should be the same for readability.
	 *
	*/
	class Effect
	{
	public:
		Effect();

		void Init(Renderer* renderer);

		virtual void CreateDescriptorPool(Device* device) = 0;
		virtual void CreateVertexDescription(Device* device) = 0;
		virtual void CreatePipelineInterface(Device* device) = 0;
		virtual void CreateDescriptorSets(Device* device) = 0;
		virtual void CreatePipeline(Renderer* renderer) = 0;
		virtual void UpdateMemory(Device* device) = 0;

		VkPipelineLayout GetPipelineLayout();
		DescriptorSetLayout GetDescriptorSetLayout(uint32_t descriptorSet);
		Pipeline2* GetPipeline();
		DescriptorPool* GetDescriptorPool();

	protected:
		DescriptorPool* mDescriptorPool;
		PipelineInterface mPipelineInterface;
		VertexDescription* mVertexDescription;
		Pipeline2* mPipeline;
	};
}
