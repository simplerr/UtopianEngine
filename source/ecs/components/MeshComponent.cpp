#include "MeshComponent.h"
#include "vulkan/StaticModel.h"

namespace ECS
{
	MeshComponent::MeshComponent(std::string filename, Utopian::Vk::PipelineType pipeline)
		: Component(MESH_COMPONENT)
	{
		mFilename = filename;
		SetPipeline(pipeline);
		SetModel(nullptr);
	}

	void MeshComponent::SetModel(Utopian::Vk::StaticModel* model)
	{
		mModel = model;
	}

	void MeshComponent::SetPipeline(Utopian::Vk::PipelineType pipeline)
	{
		mPipeline = pipeline;
	}

	Utopian::Vk::PipelineType MeshComponent::GetPipeline()
	{
		return mPipeline;
	}

	Utopian::Vk::StaticModel* MeshComponent::GetModel()
	{
		return mModel;
	}

	Utopian::BoundingBox MeshComponent::GetBoundingBox()
	{
		return mModel->GetBoundingBox();
	}

	std::string MeshComponent::GetFilename()
	{
		return mFilename;
	}
}