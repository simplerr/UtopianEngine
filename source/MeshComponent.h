#pragma once
#include <string>
#include "Component.h"

namespace ECS
{
	// Static mesh
	class MeshComponent : public Component
	{
	public:
		MeshComponent(std::string filename);
	private:
		std::string mFilename;
		// Material
		// Bounding box (gets updated)

	};
}
