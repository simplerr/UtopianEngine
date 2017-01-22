#include "MeshComponent.h"

namespace ECS
{
	MeshComponent::MeshComponent(std::string filename, Pipeline pipeline)
		: Component(MESH_COMPONENT)
	{
		mFilename = filename;
		mPipeline = pipeline;
	}

	ECS::Pipeline MeshComponent::GetPipeline()
	{
		return mPipeline;
	}

	std::string MeshComponent::GetFilename()
	{
		return mFilename;
	}
}