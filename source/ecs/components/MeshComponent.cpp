#include "MeshComponent.h"
#include "vulkan/StaticModel.h"

namespace ECS
{
	MeshComponent::MeshComponent(std::string filename, Vulkan::PipelineType pipeline)
		: Component(MESH_COMPONENT)
	{
		mFilename = filename;
		SetPipeline(pipeline);
		SetModel(nullptr);
	}

	void MeshComponent::SetModel(Vulkan::StaticModel* model)
	{
		mModel = model;
	}

	void MeshComponent::SetPipeline(Vulkan::PipelineType pipeline)
	{
		mPipeline = pipeline;
	}

	Vulkan::PipelineType MeshComponent::GetPipeline()
	{
		return mPipeline;
	}

	Vulkan::StaticModel* MeshComponent::GetModel()
	{
		return mModel;
	}

	Vulkan::BoundingBox MeshComponent::GetBoundingBox()
	{
		return mModel->GetBoundingBox();
	}

	std::string MeshComponent::GetFilename()
	{
		return mFilename;
	}
}