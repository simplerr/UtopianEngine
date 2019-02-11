#include "vulkan/GBufferEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/Vertex.h"

namespace Utopian::Vk
{
	GBufferEffect::GBufferEffect(Device* device, RenderPass* renderPass)
		: Effect(device, renderPass)
	{
		ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/gbuffer/gbuffer.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/gbuffer/gbuffer.frag";
		SetShaderCreateInfo(shaderCreateInfo);

		GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_NONE;

		CreatePipeline();

		viewProjectionBlock.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		BindUniformBuffer("UBO_viewProjection", &viewProjectionBlock);
	}

	void GBufferEffect::UpdateMemory()
	{
		viewProjectionBlock.UpdateMemory();
	}

	void GBufferEffect::SetCameraData(glm::mat4 view, glm::mat4 projection)
	{
		viewProjectionBlock.data.view = view;
		viewProjectionBlock.data.projection = projection;
		UpdateMemory();
	}
}