#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include "vulkan/Mesh.h"
#include "Collision.h"
#include "Vertex.h"

using namespace glm;

namespace VulkanLib
{
	class Device;


	class StaticModel
	{
	public:
		StaticModel();
		~StaticModel();

		void AddMesh(Mesh* mesh);
		void Init(Device* device);		// Gets called in ModelLoader::LoadModel()

		int GetNumIndices();
		int GetNumVertics();
		BoundingBox GetBoundingBox();

		std::vector<Mesh*> mMeshes;
	private:
		uint32_t mIndicesCount;
		uint32_t mVerticesCount;

		BoundingBox mBoundingBox;
	};
}	// VulkanLib namespace