#include "vulkan/MarchingCubesEffect.h"
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
	MarchingCubesEffect::MarchingCubesEffect()
	{
	}

	void MarchingCubesEffect::CreateDescriptorPool(Device* device)
	{
		mDescriptorPool = new Vulkan::DescriptorPool(device);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2);
		mDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NUM_MAX_STORAGE_BUFFERS); // NOTE:
		mDescriptorPool->Create();
	}

	void MarchingCubesEffect::CreateVertexDescription(Device* device)
	{
	}

	void MarchingCubesEffect::CreatePipelineInterface(Device* device)
	{
		// Descriptor set 0
		mPipelineInterface.AddUniformBuffer(SET_0, 1, VK_SHADER_STAGE_COMPUTE_BIT);
		mPipelineInterface.AddCombinedImageSampler(SET_0, 0, VK_SHADER_STAGE_COMPUTE_BIT);
		mPipelineInterface.AddCombinedImageSampler(SET_0, 2, VK_SHADER_STAGE_COMPUTE_BIT);

		// Descriptor set 1
		mPipelineInterface.AddStorageBuffer(SET_1, 0, VK_SHADER_STAGE_COMPUTE_BIT);
		mPipelineInterface.AddStorageBuffer(SET_1, 1, VK_SHADER_STAGE_COMPUTE_BIT);

		mPipelineInterface.AddPushConstantRange(sizeof(PushConstantBlock), VK_SHADER_STAGE_COMPUTE_BIT);

		mPipelineInterface.CreateLayouts(device);
	}

	void MarchingCubesEffect::CreateDescriptorSets(Device* device)
	{
		// Create the descriptor here, it's data needs to be set in Terrain.cpp
		ubo.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		DescriptorSetLayout setLayout0 = mPipelineInterface.GetDescriptorSetLayout(SET_0);
		mDescriptorSet0 = new Vulkan::DescriptorSet(device, &setLayout0, mDescriptorPool);
		mDescriptorSet0->AllocateDescriptorSets();
		mDescriptorSet0->BindUniformBuffer(1, &ubo.GetDescriptor());
		mDescriptorSet0->BindCombinedImage(0, &edgeTableTex->GetTextureDescriptorInfo());
		mDescriptorSet0->BindCombinedImage(2, &triangleTableTex->GetTextureDescriptorInfo());
		mDescriptorSet0->UpdateDescriptorSets();
	}

	void MarchingCubesEffect::CreatePipeline(Renderer* renderer)
	{
		Vulkan::Shader* computeShader = renderer->mShaderManager->CreateComputeShader("data/shaders/marching_cubes/marching_cubes.comp.spv");
		mComputePipeline = new Vulkan::ComputePipeline(renderer->GetDevice(), &mPipelineInterface, computeShader);
		mComputePipeline->Create();
	}

	void MarchingCubesEffect::UpdateMemory(Device* device)
	{
		ubo.UpdateMemory(device->GetVkDevice());
	}

	ComputePipeline* MarchingCubesEffect::GetComputePipeline()
	{
		return mComputePipeline;
	}

	void MarchingCubesEffect::UniformBuffer::UpdateMemory(VkDevice device)
	{
		// Map uniform buffer and update it
		uint8_t *mapped;
		mBuffer->MapMemory(0, sizeof(data), 0, (void**)&mapped);
		memcpy(mapped, &data, sizeof(data));
		mBuffer->UnmapMemory();
	}

	int MarchingCubesEffect::UniformBuffer::GetSize()
	{
		return sizeof(data);
	}
}