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

		VkPipelineLayout GetPipelineLayout();
		Pipeline2* GetPipeline();

	protected:
		DescriptorPool* mDescriptorPool;
		PipelineInterface mPipelineInterface;
		VertexDescription* mVertexDescription;
		Pipeline2* mPipeline;
	};
}
