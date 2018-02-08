#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/ShaderManager.h"

namespace Utopian::Vk
{
	ComputePipeline::ComputePipeline(Device* device, PipelineInterface* pipelineInterface, Shader* shader)
		: Handle(device, vkDestroyPipeline)
	{
		mShader = shader;
		mPipelineInterface = pipelineInterface;
	}

	void ComputePipeline::Create()
	{
		// Marching cubes pipeline
		VkComputePipelineCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		createInfo.layout = mPipelineInterface->GetPipelineLayout();
		createInfo.stage = mShader->shaderStages[0];
		vkCreateComputePipelines(GetDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &mHandle);
	}
}