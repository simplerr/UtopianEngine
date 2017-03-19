#pragma once

#include <vulkan/vulkan.h>
#include "Handle.h"

namespace Vulkan
{
	class PipelineLayout;
	class Device;
	class Shader;

	class ComputePipeline : public Handle<VkPipeline>
	{
	public:
		ComputePipeline(Device* device, PipelineLayout* pipelineLayout, Shader* shader);

		void Create();
	private:
		PipelineLayout* mPipelineLayout = nullptr;
		Shader* mShader = nullptr;
	};
}