#include "MeshComponent.h"
#include "vulkan/StaticModel.h"

namespace ECS
{
	MeshComponent::MeshComponent(std::string filename, VulkanLib::PipelineType pipeline)
		: Component(MESH_COMPONENT)
	{
		mFilename = filename;
		SetPipeline(pipeline);
		SetModel(nullptr);
	}

	void MeshComponent::SetModel(VulkanLib::StaticModel* model)
	{
		mModel = model;
	}

	void MeshComponent::SetPipeline(VulkanLib::PipelineType pipeline)
	{
		mPipeline = pipeline;
	}

	VulkanLib::PipelineType MeshComponent::GetPipeline()
	{
		return mPipeline;
	}

	VulkanLib::StaticModel* MeshComponent::GetModel()
	{
		return mModel;
	}

	VulkanLib::BoundingBox MeshComponent::GetBoundingBox()
	{
		return mModel->GetBoundingBox();
	}

	std::string MeshComponent::GetFilename()
	{
		return mFilename;
	}
}