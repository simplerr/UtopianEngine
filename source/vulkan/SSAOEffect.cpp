#include "vulkan/SSAOEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/Renderer.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/handles/Effect.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/Vertex.h"
#include "core/renderer/Light.h"
#include <memory>

namespace Utopian::Vk
{
	SSAOEffect::SSAOEffect(Device* device, RenderPass* renderPass)
		: Effect(device, renderPass, "data/shaders/common/fullscreen.vert", "data/shaders/ssao/ssao.frag")
	{
		mPipeline->rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		mPipeline->depthStencilState.depthTestEnable = VK_TRUE;

		// Vertices generated in fullscreen.vert are in clockwise order
		mPipeline->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mPipeline->rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		CreatePipeline();

		cameraBlock.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		settingsBlock.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		// Note: Perhaps this should be moved to separate class instead
		BindUniformBuffer("UBO", &cameraBlock);
		BindUniformBuffer("UBO_settings", &settingsBlock);
	}

	void SSAOEffect::UpdateMemory()
	{
		cameraBlock.UpdateMemory();
		settingsBlock.UpdateMemory();
	}

	void SSAOEffect::BindGBuffer(Image* positionImage, Image* normalViewImage, Image* albedoImage, Sampler* sampler)
	{
		BindCombinedImage("positionSampler", positionImage, sampler);
		BindCombinedImage("normalSampler", normalViewImage, sampler);
		BindCombinedImage("albedoSampler", albedoImage, sampler);
	}
	
	void SSAOEffect::SetCameraData(glm::mat4 view, glm::mat4 projection, glm::vec4 eyePos)
	{
		cameraBlock.data.view = view;
		cameraBlock.data.projection = projection;
		cameraBlock.data.eyePos = eyePos;
		UpdateMemory();
	}

	void SSAOEffect::SetSettings(float radius, float bias)
	{
		settingsBlock.data.radius = radius;
		settingsBlock.data.bias = bias;
		UpdateMemory();
	}
}