#include "vulkan/Effect.h"
#include "vulkan/Renderer.h"
#include "vulkan/Device.h"

namespace Vulkan
{
	Effect::Effect()
	{

	}

	void Effect::Init(Renderer* renderer)
	{
		CreateDescriptorPool(renderer->GetDevice());
		CreateVertexDescription(renderer->GetDevice());
		CreatePipelineInterface(renderer->GetDevice());
		CreateDescriptorSets(renderer->GetDevice());
		CreatePipeline(renderer); // To access the shader manager.
	}

	VkPipelineLayout Effect::GetPipelineLayout()
	{
		return mPipelineInterface.GetPipelineLayout();
	}

	Pipeline2* Effect::GetPipeline()
	{
		return mPipeline;
	}
}