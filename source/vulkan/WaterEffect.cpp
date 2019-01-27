#include "vulkan/WaterEffect.h"
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
	WaterEffect::WaterEffect()
	{
		per_frame_vs.data.moveFactor = 0.0f;
	}

	void WaterEffect::CreateDescriptorPool(Device* device)
	{
		mDescriptorPool = new Utopian::Vk::DescriptorPool(device);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4);
		mDescriptorPool->Create();
	}

	void WaterEffect::CreateVertexDescription(Device* device)
	{
		mVertexDescription = Vertex::GetDescription();
	}

	void WaterEffect::CreatePipelineInterface(Device* device)
	{
		// Descriptor set 0
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_0, VK_SHADER_STAGE_VERTEX_BIT);						// per_frame_vs UBO
		mPipelineInterface.AddCombinedImageSampler(SET_0, BINDING_1, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddCombinedImageSampler(SET_0, BINDING_2, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddCombinedImageSampler(SET_0, BINDING_3, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddPushConstantRange(sizeof(PushConstantBlock), VK_SHADER_STAGE_VERTEX_BIT);
		mPipelineInterface.CreateLayouts(device);
	}

	void WaterEffect::CreateDescriptorSets(Device* device)
	{
		per_frame_vs.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		per_frame_ps.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		mDescriptorSet0 = new Utopian::Vk::DescriptorSet(device, mPipelineInterface.GetDescriptorSetLayout(SET_0), mDescriptorPool);
		mDescriptorSet0->BindUniformBuffer(BINDING_0, per_frame_vs.GetDescriptor());
		mDescriptorSet0->UpdateDescriptorSets();
	}

	void WaterEffect::CreatePipeline(Device* device, RenderPass* renderPass)
	{
		Shader* shader = gShaderFactory().CreateShader("data/shaders/water/water.vert.spv", "data/shaders/water/water.frag.spv");

		Pipeline2*  pipeline = new Pipeline2(device, renderPass, mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline->Create();
		mPipelines[0] = pipeline;
	}

	void WaterEffect::UpdateMemory()
	{
		per_frame_vs.UpdateMemory();
		per_frame_ps.UpdateMemory();
	}

	void WaterEffect::UniformBufferVS::UpdateMemory()
	{
		// Map uniform buffer and update it
		uint8_t *mapped;
		mBuffer->MapMemory(0, sizeof(data), 0, (void**)&mapped);
		memcpy(mapped, &data, sizeof(data));
		mBuffer->UnmapMemory();
	}

	int WaterEffect::UniformBufferVS::GetSize()
	{
		return sizeof(data);
	}

	void WaterEffect::UniformBufferPS::UpdateMemory()
	{
		// Map uniform buffer and update it
		uint8_t *mapped;
		mBuffer->MapMemory(0, sizeof(data), 0, (void**)&mapped);
		memcpy(mapped, &data, sizeof(data));
		mBuffer->UnmapMemory();
	}

	int WaterEffect::UniformBufferPS::GetSize()
	{
		return sizeof(data);
	}
}