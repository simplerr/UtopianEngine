#include "vulkan/ColorEffect.h"
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
	ColorEffect::ColorEffect(Device* device, RenderPass* renderPass)
		: Effect(device, renderPass)
	{
		ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/color/color.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/color/color.frag";
		SetShaderCreateInfo(shaderCreateInfo);

		// Note: Uncommented since the DebugJob is calling this instead right now
		//CreatePipeline();
	}

	void ColorEffect::BindDeferredOutput(Image* deferredImage, Sampler* sampler)
	{
		//BindCombinedImage("samplerDeferred", deferredImage, sampler);
	}
}