#include "StaticModel.h"
#include "VulkanDebug.h"
#include "Device.h"

namespace Vulkan
{
	StaticModel::StaticModel()
	{

	}

	StaticModel::~StaticModel()
	{

	}

	void StaticModel::AddMesh(Mesh* mesh)
	{
		mMeshes.push_back(mesh);
	}

	void StaticModel::Init(Device* device)
	{
		std::vector<Vertex> vertexVector;
		std::vector<uint32_t> indexVector;

		for (int meshId = 0; meshId < mMeshes.size(); meshId++)
		{
			for (int i = 0; i < mMeshes[meshId]->vertexVector.size(); i++)
			{
				Vertex vertex = mMeshes[meshId]->vertexVector[i];
				vertexVector.push_back(vertex);
			}

			for (int i = 0; i < mMeshes[meshId]->indexVector.size(); i++)
			{
				indexVector.push_back(mMeshes[meshId]->indexVector[i]);
			}
		}

		mVerticesCount = vertexVector.size();
		mIndicesCount = indexVector.size();

		mBoundingBox.Init(vertexVector);
	}

	int StaticModel::GetNumIndices()
	{
		return mIndicesCount;
	}

	int StaticModel::GetNumVertics()
	{
		return mVerticesCount;
	}

	BoundingBox StaticModel::GetBoundingBox()
	{
		return mBoundingBox;
	}
}	// VulkanLib namespace