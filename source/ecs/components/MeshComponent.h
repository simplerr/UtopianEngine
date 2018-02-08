#pragma once

#include <string>
#include "Component.h"
#include "Collision.h"

namespace Utopian::Vk
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
		MeshComponent(std::string filename, Utopian::Vk::PipelineType pipeline);

		void SetModel(Utopian::Vk::StaticModel* model);
		void SetPipeline(Utopian::Vk::PipelineType pipeline);

		Utopian::Vk::PipelineType GetPipeline();
		Utopian::Vk::StaticModel* GetModel();
		Utopian::Vk::BoundingBox GetBoundingBox();

		std::string GetFilename();

	private:
		Utopian::Vk::PipelineType mPipeline;
		Utopian::Vk::StaticModel* mModel;
		std::string mFilename;

		// Material
		// Pipeline
		// Bounding box (gets updated)
	};
}
