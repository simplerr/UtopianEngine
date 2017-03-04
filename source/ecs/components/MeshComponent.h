#pragma once

#include <string>
#include "Component.h"
#include "Collision.h"

namespace Vulkan
{
	enum PipelineType;
	class StaticModel;
}

namespace ECS
{

	// Static mesh
	class MeshComponent : public Component
	{
	public:
		MeshComponent(std::string filename, Vulkan::PipelineType pipeline);

		void SetModel(Vulkan::StaticModel* model);
		void SetPipeline(Vulkan::PipelineType pipeline);

		Vulkan::PipelineType GetPipeline();
		Vulkan::StaticModel* GetModel();
		Vulkan::BoundingBox GetBoundingBox();

		std::string GetFilename();

	private:
		Vulkan::PipelineType mPipeline;
		Vulkan::StaticModel* mModel;
		std::string mFilename;

		// Material
		// Pipeline
		// Bounding box (gets updated)
	};
}
