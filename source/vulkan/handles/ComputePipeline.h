#pragma once

#include "vulkan/VulkanInclude.h"
#include "Handle.h"

namespace Utopian::Vk
{
	/** Todo: Note: Legacy. */
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