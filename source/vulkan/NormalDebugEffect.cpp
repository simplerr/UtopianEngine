#include "vulkan/NormalDebugEffect.h"
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

namespace Utopian::Vk
{
	NormalDebugEffect::NormalDebugEffect()
	{
	}

	void NormalDebugEffect::CreateDescriptorPool(Device* device)
	{
		mDescriptorPool = new Utopian::Vk::DescriptorPool(device);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
		mDescriptorPool->Create();
	}

	void NormalDebugEffect::CreateVertexDescription(Device* device)
	{
		mVertexDescription = Vertex::GetDescription();
	}

	void NormalDebugEffect::CreatePipelineInterface(Device* device)
	{
		// Descriptor set 0
		mPipelineInterface.AddUniformBuffer(SET_0, BINDING_0, VK_SHADER_STAGE_GEOMETRY_BIT);						
		mPipelineInterface.AddPushConstantRange(sizeof(PushConstantBlock), VK_SHADER_STAGE_GEOMETRY_BIT);
		mPipelineInterface.CreateLayouts(device);
	}

	void NormalDebugEffect::CreateDescriptorSets(Device* device)
	{
		per_frame_gs.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		mDescriptorSet0 = new Utopian::Vk::DescriptorSet(device, mPipelineInterface.GetDescriptorSetLayout(SET_0), mDescriptorPool);
		mDescriptorSet0->BindUniformBuffer(BINDING_0, per_frame_gs.GetDescriptor());
		mDescriptorSet0->UpdateDescriptorSets();
	}

	void NormalDebugEffect::CreatePipeline(Renderer* renderer)
	{
		Utopian::Vk::Shader* shader = renderer->mShaderManager->CreateShader("data/shaders/normal_debug/base.vert.spv", "data/shaders/normal_debug/base.frag.spv", "data/shaders/normal_debug/normaldebug.geom.spv");

		Pipeline2*  pipeline = new Pipeline2(renderer->GetDevice(), renderer->GetRenderPass(), mVertexDescription, shader);
		pipeline->SetPipelineInterface(&mPipelineInterface);
		pipeline->mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline->Create();
		mPipelines[0] = pipeline;
	}

	void NormalDebugEffect::UpdateMemory(Device* device)
	{
		per_frame_gs.UpdateMemory();
	}

	void NormalDebugEffect::UniformBufferGS::UpdateMemory()
	{
		// Map uniform buffer and update it
		uint8_t *mapped;
		mBuffer->MapMemory(0, sizeof(data), 0, (void**)&mapped);
		memcpy(mapped, &data, sizeof(data));
		mBuffer->UnmapMemory();
	}

	int NormalDebugEffect::UniformBufferGS::GetSize()
	{
		return sizeof(data);
	}
}