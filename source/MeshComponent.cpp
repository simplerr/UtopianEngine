#include "MeshComponent.h"

namespace ECS
{
	MeshComponent::MeshComponent(std::string filename)
		: Component(MESH_COMPONENT)
	{
		mFilename = filename;
	}
}