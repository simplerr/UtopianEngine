#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/ShaderManager.h"

namespace Vulkan
{
	ComputePipeline::ComputePipeline(Device* device, PipelineLayout* pipelineLayout, Shader* shader)
		: Handle(device, vkDestroyPipeline)
	{
		mShader = shader;
		mPipelineLayout = pipelineLayout;
	}

	void ComputePipeline::Create()
	{
		// Marching cubes pipeline
		VkComputePipelineCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		createInfo.layout = mPipelineLayout->GetVkHandle();
		createInfo.stage = mShader->shaderStages[0];
		vkCreateComputePipelines(GetDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &mHandle);
	}
}