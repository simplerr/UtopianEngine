#pragma once

#include <vulkan/vulkan.h>
#include "Handle.h"

namespace Vulkan
{
	class PipelineInterface;
	class Device;
	class Shader;

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