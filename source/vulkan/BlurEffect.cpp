#include "vulkan/BlurEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/Renderer.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/Vertex.h"
#include "core/renderer/Light.h"
#include "core/renderer/SceneJobs.h"

namespace Utopian::Vk
{
	BlurEffect::BlurEffect(Device* device, RenderPass* renderPass)
		: Effect(device, renderPass, "data/shaders/blur/blur.vert", "data/shaders/blur/blur.frag")
	{
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