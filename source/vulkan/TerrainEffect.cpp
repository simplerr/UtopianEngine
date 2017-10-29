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
		SetPipeline(PipelineType2::WIREFRAME);
	}

	void TerrainEffect::CreateDescriptorPool(Device* device)
	{
		mDescriptorPool = new Vulkan::DescriptorPool(device);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
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
		mPipelineInterface.AddCombinedImageSampler(SET_0, BINDING_2, VK_SHADER_STAGE_FRAGMENT_BIT);
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
		mDescriptorSet0->BindCombinedImage(BINDING_2, &texture3d->GetTextureDescriptorInfo());
		mDescriptorSet0->UpdateDescriptorSets();
	}

	void TerrainEffect::CreatePipeline(Renderer* renderer)
	{
		Vulkan::Shader* shader = renderer->mShaderManager->CreateShader("data/shaders/terrain/terrain.vert.spv", "data/shaders/terrain/terrain.frag.spv");

		Pipeline2* pipeline = new Vulkan::Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		pipeline->Create();
		mPipelines[PipelineType2::WIREFRAME] = pipeline;

		Pipeline2* pipeline1 = new Vulkan::Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);
		pipeline1->SetPipelineInterface(&mPipelineInterface);
		pipeline1->mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline1->Create();
		mPipelines[PipelineType2::SOLID] = pipeline1;
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