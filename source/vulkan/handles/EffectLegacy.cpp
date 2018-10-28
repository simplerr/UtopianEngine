#include "vulkan/EffectLegacy.h"
#include "vulkan/Renderer.h"
#include "vulkan/Device.h"

namespace Utopian::Vk
{
	EffectLegacy::EffectLegacy()
	{
		mActivePipeline = 0;
	}

	void EffectLegacy::Init(Renderer* renderer)
	{
		CreateDescriptorPool(renderer->GetDevice());
		CreateVertexDescription(renderer->GetDevice());
		CreatePipelineInterface(renderer->GetDevice());
		CreatePipeline(renderer); // To access the shader manager.
		CreateDescriptorSets(renderer->GetDevice());
	}

	void EffectLegacy::SetPipeline(uint32_t pipelineType)
	{
		mActivePipeline = pipelineType;
	}

	VkPipelineLayout EffectLegacy::GetPipelineLayout()
	{
		return mPipelineInterface.GetPipelineLayout();
	}

	DescriptorSetLayout* EffectLegacy::GetDescriptorSetLayout(uint32_t descriptorSet)
	{
		return mPipelineInterface.GetDescriptorSetLayout(descriptorSet);
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