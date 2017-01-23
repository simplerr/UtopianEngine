#pragma once

#include <string>
#include "Component.h"

namespace VulkanLib
{
	enum PipelineType;
}

namespace ECS
{

	// Static mesh
	class MeshComponent : public Component
	{
	public:
		MeshComponent(std::string filename, VulkanLib::PipelineType pipeline);

		void SetPipeline(VulkanLib::PipelineType pipeline);
		VulkanLib::PipelineType GetPipeline();
		std::string GetFilename();
	private:
		VulkanLib::PipelineType mPipeline;
		std::string mFilename;

		// Material
		// Pipeline
		// Bounding box (gets updated)
	};
}
