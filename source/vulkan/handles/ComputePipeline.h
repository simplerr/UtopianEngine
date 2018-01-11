#pragma once

#include <vulkan/vulkan.h>
#include "vulkan/VulkanInclude.h"
#include "Handle.h"

namespace Vulkan
{
	class ComputePipeline : public Handle<VkPipeline>
	{
	public:
		ComputePipeline(Device* device, PipelineInterface* pipelineInterface, Shader* shader);

		void Create();
	private:
		PipelineInterface* mPipelineInterface = nullptr;
		Shader* mShader = nullptr;
	};
}