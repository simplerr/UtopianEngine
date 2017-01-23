#include "MeshComponent.h"

namespace ECS
{
	MeshComponent::MeshComponent(std::string filename, VulkanLib::PipelineType pipeline)
		: Component(MESH_COMPONENT)
	{
		mFilename = filename;
		SetPipeline(pipeline);
	}

	void MeshComponent::SetPipeline(VulkanLib::PipelineType pipeline)
	{
		mPipeline = pipeline;
	}

	VulkanLib::PipelineType MeshComponent::GetPipeline()
	{
		return mPipeline;
	}

	std::string MeshComponent::GetFilename()
	{
		return mFilename;
	}
}