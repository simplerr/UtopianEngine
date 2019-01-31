#include "vulkan/EffectLegacy.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/handles/Device.h"

namespace Utopian::Vk
{
	EffectLegacy::EffectLegacy()
	{
		mActivePipeline = 0;
	}

	void EffectLegacy::Init(Device* device, RenderPass* renderPass)
	{
		CreateDescriptorPool(device);
		CreateVertexDescription(device);
		CreatePipelineInterface(device);
		CreatePipeline(device, renderPass);
		CreateDescriptorSets(device);
	}

	void EffectLegacy::SetPipeline(uint32_t pipelineType)
	{
		mActivePipeline = pipelineType;
	}

	VkPipelineLayout EffectLegacy::GetPipelineLayout() const
	{
		return mPipelineInterface->GetPipelineLayout();
	}

	const DescriptorSetLayout* EffectLegacy::GetDescriptorSetLayout(uint32_t descriptorSet)
	{
		return mPipelineInterface->GetDescriptorSetLayout(descriptorSet);
	}

	Pipeline2* EffectLegacy::GetPipeline(uint32_t variation)
	{
		return mPipelines[variation];
	}

	DescriptorPool* EffectLegacy::GetDescriptorPool()
	{
		return mDescriptorPool;
	}

	VertexDescription EffectLegacy::GetVertexDescription()
	{
		return mVertexDescription;
	}
}