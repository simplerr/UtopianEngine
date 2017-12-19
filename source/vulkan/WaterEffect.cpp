#include "vulkan/WaterEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/Renderer.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/ShaderManager.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/handles/ComputePipeline.h"
#include "vulkan/PipelineInterface.h"
#include "vulkan/Vertex.h"

namespace Vulkan
{
	WaterEffect::WaterEffect()
	{
	}

	void WaterEffect::CreateDescriptorPool(Device* device)
	{
		mDescriptorPool = new Vulkan::DescriptorPool(device);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4);
		mDescriptorPool->Create();
	}

	void WaterEffect::CreateVertexDescription(Device* device)
	{
		// First tell Vulkan about how large each vertex is, the binding ID and the inputRate
		mVertexDescription = new Vulkan::VertexDescription();
		mVertexDescription->AddBinding(BINDING_0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);

		// We need to tell Vulkan about the memory layout for each attribute
		// 5 attributes: position, normal, texture coordinates, tangent and color
		// See Vertex struct
		mVertexDescription->AddAttribute(BINDING_0, Vec3Attribute());	// Location 0 : Position
		mVertexDescription->AddAttribute(BINDING_0, Vec3Attribute());	// Location 1 : Color
		mVertexDescription->AddAttribute(BINDING_0, Vec3Attribute());	// Location 2 : Normal
		mVertexDescription->AddAttribute(BINDING_0, Vec2Attribute());	// Location 3 : Texture
		mVertexDescription->AddAttribute(BINDING_0, Vec4Attribute());	// Location 4 : Tangent
	}

	void WaterEffect::CreatePipelineInterface(Device* device)
	{
		// Descriptor set 0
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_0, VK_SHADER_STAGE_VERTEX_BIT);						// per_frame_vs UBO
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_1, VK_SHADER_STAGE_FRAGMENT_BIT);					// per_frame_ps UBO
		mPipelineInterface.AddCombinedImageSampler(SET_0, BINDING_2, VK_SHADER_STAGE_FRAGMENT_BIT);
		mPipelineInterface.AddPushConstantRange(sizeof(PushConstantBlock), VK_SHADER_STAGE_VERTEX_BIT);
		mPipelineInterface.CreateLayouts(device);
	}

	void WaterEffect::CreateDescriptorSets(Device* device)
	{
		per_frame_vs.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		per_frame_ps.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		mDescriptorSet0 = new Vulkan::DescriptorSet(device, mPipelineInterface.GetDescriptorSetLayout(SET_0), mDescriptorPool);
		mDescriptorSet0->BindUniformBuffer(BINDING_0, &per_frame_vs.GetDescriptor());
		mDescriptorSet0->UpdateDescriptorSets();
	}

	void WaterEffect::CreatePipeline(Renderer* renderer)
	{
		Shader* shader = renderer->mShaderManager->CreateShader("data/shaders/water/water.vert.spv", "data/shaders/water/water.frag.spv");

		Pipeline2*  pipeline = new Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline->Create();
		mPipelines[0] = pipeline;
	}

	void WaterEffect::UpdateMemory(Device* device)
	{
		per_frame_vs.UpdateMemory(device->GetVkDevice());
		per_frame_ps.UpdateMemory(device->GetVkDevice());
	}

	void WaterEffect::UniformBufferVS::UpdateMemory(VkDevice device)
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

	void WaterEffect::UniformBufferPS::UpdateMemory(VkDevice device)
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