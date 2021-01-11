#pragma once

#include <glm/glm.hpp>
#include "vulkan/VulkanPrerequisites.h"

namespace Utopian
{
	class Renderer;
	struct SceneInfo;

	class InstancingManager
	{
	public:
		InstancingManager(Renderer* renderer);
		~InstancingManager();

		void AddInstancedAsset(uint32_t assetId, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, bool animated = false, bool castShadow = false);
		void RemoveInstancesWithinRadius(uint32_t assetId, glm::vec3 position, float radius);
		void BuildAllInstances();
		void ClearInstanceGroups();
		void UpdateInstanceAltitudes();
		void SaveInstancesToFile(const std::string& filename);
		void LoadInstancesFromFile(const std::string& filename);
	private:
		SceneInfo* mSceneInfo;
		Vk::Device* mDevice;
	};
}
