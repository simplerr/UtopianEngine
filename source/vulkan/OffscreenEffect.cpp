#include "vulkan/OffscreenEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/Renderer.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/ShaderManager.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/PipelineInterface.h"

namespace Vulkan
{
	OffscreenEffect::OffscreenEffect()
	{
	}

	void OffscreenEffect::CreateDescriptorPool(Device* device)
	{
		mDescriptorPool = new Vulkan::DescriptorPool(device);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
		mDescriptorPool->Create();
	}

	void OffscreenEffect::CreateVertexDescription(Device* device)
	{
		mVertexDescription = new Vulkan::VertexDescription();
		mVertexDescription->AddBinding(BINDING_0, sizeof(BasicVertex), VK_VERTEX_INPUT_RATE_VERTEX);
		mVertexDescription->AddAttribute(BINDING_0, Vulkan::Vec4Attribute());	// InPosL
		mVertexDescription->AddAttribute(BINDING_0, Vulkan::Vec4Attribute());	// InNormal	
	}

	void OffscreenEffect::CreatePipelineInterface(Device* device)
	{
		// Descriptor set 0
		mPipelineInterface.AddCombinedImageSampler(SET_0, BINDING_0, VK_SHADER_STAGE_COMPUTE_BIT);
		mPipelineInterface.AddPushConstantRange(sizeof(PushConstantBlock), VK_SHADER_STAGE_COMPUTE_BIT);
		mPipelineInterface.CreateLayouts(device);
	}

	void OffscreenEffect::CreateDescriptorSets(Device* device)
	{
	}

	void OffscreenEffect::CreatePipeline(Renderer* renderer)
	{
		Shader* shader = renderer->mShaderManager->CreateShader("data/shaders/offscreen/offscreen.vert.spv", "data/shaders/offscreen/offscreen.frag.spv");

		Pipeline2*  pipeline = new Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline->Create();
		mPipelines[0] = pipeline;
	}

	void OffscreenEffect::UpdateMemory(Device* device)
	{
		per_frame_vs.UpdateMemory(device->GetVkDevice());
	}

	void OffscreenEffect::UniformBufferVS::UpdateMemory(VkDevice device)
	{
		// Map uniform buffer and update it
		uint8_t *mapped;
		mBuffer->MapMemory(0, sizeof(data), 0, (void**)&mapped);
		memcpy(mapped, &data, sizeof(data));
		mBuffer->UnmapMemory();
	}

	int OffscreenEffect::UniformBufferVS::GetSize()
	{
		return sizeof(data);
	}
}