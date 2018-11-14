#include "vulkan/ColorEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/Renderer.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/Vertex.h"

namespace Utopian::Vk
{
	ColorEffect::ColorEffect(Device* device, RenderPass* renderPass)
		: Effect(device, renderPass, "data/shaders/color/color.vert", "data/shaders/color/color.frag")
	{
		CreatePipeline();

		viewProjectionBlock.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		BindUniformBuffer("UBO_viewProjection", &viewProjectionBlock);
	}

	void ColorEffect::UpdateMemory()
	{
		viewProjectionBlock.UpdateMemory();
	}

	void ColorEffect::SetCameraData(glm::mat4 view, glm::mat4 projection)
	{
		viewProjectionBlock.data.view = view;
		viewProjectionBlock.data.projection = projection;
		UpdateMemory();
	}

	void ColorEffect::BindDeferredOutput(Image* deferredImage, Sampler* sampler)
	{
		//BindCombinedImage("samplerDeferred", deferredImage, sampler);
	}
}