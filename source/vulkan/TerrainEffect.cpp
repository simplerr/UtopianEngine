#include "vulkan/TerrainEffect.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/PipelineLayout.h"
#include "vulkan/Renderer.h"
#include "vulkan/ShaderManager.h"
#include "vulkan/handles/Pipeline2.h"
#include "vulkan/PipelineInterface.h"

namespace Vulkan
{
	TerrainEffect::TerrainEffect()
	{
	}

	void TerrainEffect::CreateDescriptorPool(Device* device)
	{
		mDescriptorPool = new Vulkan::DescriptorPool(device);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
		mDescriptorPool->Create();
	}

	void TerrainEffect::CreateVertexDescription(Device* device)
	{
		mVertexDescription = new Vulkan::VertexDescription();
		mVertexDescription->AddBinding(BINDING_0, sizeof(BasicVertex), VK_VERTEX_INPUT_RATE_VERTEX);					
		mVertexDescription->AddAttribute(BINDING_0, Vulkan::Vec4Attribute());	// InPosL
		mVertexDescription->AddAttribute(BINDING_0, Vulkan::Vec4Attribute());	// InNormal	
	}

	void TerrainEffect::CreatePipelineInterface(Device* device)
	{
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_0, VK_SHADER_STAGE_VERTEX_BIT);						// per_frame_vs UBO
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_1, VK_SHADER_STAGE_FRAGMENT_BIT);					// per_frame_ps UBO
		mPipelineInterface.AddPushConstantRange(sizeof(PushConstantBasicBlock), VK_SHADER_STAGE_VERTEX_BIT);	// pushConsts
		mPipelineInterface.CreateLayouts(device);
	}

	void TerrainEffect::CreateDescriptorSets(Device* device)
	{
		per_frame_vs.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		per_frame_ps.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		VkDescriptorSetLayout setLayout0 = mPipelineInterface.GetVkDescriptorSetLayout(SET_0);
		mDescriptorSet0 = new Vulkan::DescriptorSet(device, setLayout0, mDescriptorPool);
		mDescriptorSet0->AllocateDescriptorSets();
		mDescriptorSet0->BindUniformBuffer(BINDING_0, &per_frame_vs.GetDescriptor());
		mDescriptorSet0->BindUniformBuffer(BINDING_1, &per_frame_ps.GetDescriptor());
		mDescriptorSet0->UpdateDescriptorSets();
	}

	void TerrainEffect::CreatePipeline(Renderer* renderer)
	{
		Vulkan::Shader* shader = renderer->mShaderManager->CreateShader("data/shaders/terrain/terrain.vert.spv", "data/shaders/terrain/terrain.frag.spv");
		mPipeline = new Vulkan::Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);

		// EXPERIMENT
		mPipeline->SetPipelineInterface(&mPipelineInterface);
		mPipeline->mInputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		mPipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		mPipeline->Create();
	}

	void TerrainEffect::UpdateMemory(Device* device)
	{
		per_frame_vs.UpdateMemory(device->GetVkDevice());
		per_frame_ps.UpdateMemory(device->GetVkDevice());
	}

	void TerrainEffect::UniformBufferVS::UpdateMemory(VkDevice device)
	{
		// Map uniform buffer and update it
		uint8_t *mapped;
		mBuffer->MapMemory(0, sizeof(data), 0, (void**)&mapped);
		memcpy(mapped, &data, sizeof(data));
		mBuffer->UnmapMemory();
	}

	int TerrainEffect::UniformBufferVS::GetSize()
	{
		return sizeof(data);
	}

	void TerrainEffect::UniformBufferPS::UpdateMemory(VkDevice device)
	{
		// Map uniform buffer and update it
		uint8_t *mapped;
		mBuffer->MapMemory(0, sizeof(data), 0, (void**)&mapped);
		memcpy(mapped, &data, sizeof(data));
		mBuffer->UnmapMemory();
	}

	int TerrainEffect::UniformBufferPS::GetSize()
	{
		return sizeof(data);
	}
}