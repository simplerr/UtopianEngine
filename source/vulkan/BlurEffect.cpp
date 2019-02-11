#include "vulkan/BlurEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/Vertex.h"
#include "core/renderer/Light.h"

namespace Utopian::Vk
{
	BlurEffect::BlurEffect(Device* device, RenderPass* renderPass)
		: Effect(device, renderPass)
	{
		ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/common/fullscreen.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/blur/blur.frag";
		SetShaderCreateInfo(shaderCreateInfo);

		// Vertices generated in fullscreen.vert are in clockwise order
		mPipeline->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mPipeline->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		CreatePipeline();

		settingsBlock.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		BindUniformBuffer("UBO_settings", &settingsBlock);
	}

	void BlurEffect::UpdateMemory()
	{
		settingsBlock.UpdateMemory();
	}

	void BlurEffect::BindSSAOOutput(Image* ssaoImage, Sampler* sampler)
	{
		BindCombinedImage("samplerSSAO", ssaoImage, sampler);
	}
	
	void BlurEffect::SetSettings(int blurRange)
	{
		settingsBlock.data.blurRange = blurRange;
		UpdateMemory();
	}
}