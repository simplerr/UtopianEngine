#pragma once
#include <glm/glm.hpp>
#include <array>
#include "vulkan/VulkanApp.h"
#include "core/renderer/Renderable.h"
#include "core/renderer/Light.h"
#include "core/Terrain.h"

#define SHADOW_MAP_CASCADE_COUNT 4

namespace Utopian
{
	struct InstanceData
	{
		glm::mat4 world;
	};

	class InstanceGroup
	{
	public:
		InstanceGroup(uint32_t assetId);

		void AddInstance(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
		void ClearInstances();
		void BuildBuffer(Vk::Device* device);

		uint32_t GetAssetId();
		uint32_t GetNumInstances();
		Vk::Buffer* GetBuffer();
		Vk::StaticModel* GetModel();

	private:
		SharedPtr<Vk::Buffer> mInstanceBuffer;
		Vk::StaticModel* mModel;
		std::vector<InstanceData> mInstances;
		uint32_t mAssetId;
	};

	class Cascade
	{
	public:
		float splitDepth;
		glm::mat4 viewProjMatrix;
	};

	struct SceneInfo
	{
		std::vector<Renderable*> renderables;
		std::vector<Light*> lights;
		std::vector<Camera*> cameras;
		SharedPtr<Terrain> terrain;
		std::vector<SharedPtr<InstanceGroup>> instanceGroups;
		std::array<Cascade, SHADOW_MAP_CASCADE_COUNT> cascades;

		// The light that will cast shadows
		// Currently assumes that there only is one directional light in the scene
		Light* directionalLight;

		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
		glm::vec3 eyePos;
	};
}
