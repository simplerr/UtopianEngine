#pragma once

#include <string>
#include "Component.h"
#include "Collision.h"

namespace VulkanLib
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
		MeshComponent(std::string filename, VulkanLib::PipelineType pipeline);

		void SetModel(VulkanLib::StaticModel* model);
		void SetPipeline(VulkanLib::PipelineType pipeline);

		VulkanLib::PipelineType GetPipeline();
		VulkanLib::StaticModel* GetModel();
		VulkanLib::BoundingBox GetBoundingBox();

		std::string GetFilename();

	private:
		VulkanLib::PipelineType mPipeline;
		VulkanLib::StaticModel* mModel;
		std::string mFilename;

		// Material
		// Pipeline
		// Bounding box (gets updated)
	};
}
