#pragma once
#include <string>
#include "Component.h"

namespace ECS
{
	enum Pipeline
	{
		PIPELINE_BASIC
	};

	// Static mesh
	class MeshComponent : public Component
	{
	public:
		MeshComponent(std::string filename, Pipeline pipeline);

		ECS::Pipeline GetPipeline();
		std::string GetFilename();
	private:
		ECS::Pipeline mPipeline;
		std::string mFilename;

		// Material
		// Pipeline
		// Bounding box (gets updated)
	};
}
