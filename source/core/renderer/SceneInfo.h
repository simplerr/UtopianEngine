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
	class Terrain;

	struct InstanceData
	{
		glm::mat4 world;
	};

	class InstanceGroup
	{
	public:
		InstanceGroup(uint32_t assetId, bool animated = false, bool castShadows = false);

		void AddInstance(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
		void RemoveInstances();
		void RemoveInstancesWithinRadius(glm::vec3 position, float radius);
		void UpdateAltitudes(const SharedPtr<Terrain>& terrain);
		void BuildBuffer(Vk::Device* device);
		void SetAnimated(bool animated);
		void SetCastShadows(bool castShadows);

		uint32_t GetAssetId();
		uint32_t GetNumInstances();
		Vk::Buffer* GetBuffer();
		Vk::StaticModel* GetModel();
		bool IsAnimated();
		bool IsCastingShadows();

	private:
		SharedPtr<Vk::Buffer> mInstanceBuffer;
		Vk::StaticModel* mModel;
		std::vector<InstanceData> mInstances; // Uploaded to GPU
		std::vector<glm::vec3> mCachedPositions;
		uint32_t mAssetId;
		bool mAnimated;
		bool mCastShadows;
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
		SharedPtr<Vk::Buffer> im3dVertices;

		// The light that will cast shadows
		// Currently assumes that there only is one directional light in the scene
		Light* directionalLight;

		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
		glm::vec3 eyePos;
	};
}
