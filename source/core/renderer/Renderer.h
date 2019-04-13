#pragma once
#include "core/CommonBuffers.h"
#include "core/renderer/BaseJob.h"
#include "core/renderer/RenderSettings.h"
#include "core/renderer/JobGraph.h"
#include "core/Object.h"
#include "vulkan/VulkanInclude.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "utility/Module.h"
#include "utility/Common.h"
#include "utility/Timer.h"
#include <vector>

namespace Utopian
{
	/**
	 * The scene renderer that manages and renders all the nodes in the scene.
	 * Rendering is performed by executing all the jobs in the JobGraph.
	 */
	class Renderer : public Module<Renderer>
	{
	public:
		Renderer(Vk::VulkanApp* vulkanApp);
		~Renderer();

		/** Updates the cascade frustums, ImGui interface and the terrain. */
		void Update();

		/** Executes the job graph to render the scene. */
		void Render();

		/** Initialization that needs to be performed after actors are added to the scene.*/
		void PostWorldInit();

		/** Adds a Renderable to the scene. */
		void AddRenderable(Renderable* renderable);

		/** Adds a Light to the scene. */
		void AddLight(Light* light);

		/**
		 * Adds a Camera to the scene.
		 * @note Currently only one camera is supported.
		 */
		void AddCamera(Camera* camera);

		/** Removes a Renderable from the scene. */
		void RemoveRenderable(Renderable* renderable);

		/** Removes a Light from the scene. */
		void RemoveLight(Light* light);

		/** Removes a Camera from the scene. */
		void RemoveCamera(Camera* camera);

		/** Sets the main camera of the scene. */
		void SetMainCamera(Camera* camera);

		/** Returns the main camera of the scene. */
		Camera* GetMainCamera() const;

		/** Returns the Vulkan device. Note: Is this OK? */
		Vk::Device* GetDevice() const;

		/** Returns the terrain. */
		const SharedPtr<Terrain>& GetTerrain() const;

		/** Returns the configured rendering settings. */
		const RenderingSettings& GetRenderingSettings() const;

		/** Instancing experimentation. */
		void AddInstancedAsset(uint32_t assetId, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, bool animated = false, bool castShadow = false);
		void RemoveInstancesWithinRadius(uint32_t assetId, glm::vec3 position, float radius);
		void BuildAllInstances();
		void ClearInstanceGroups();

		/** Adds the buffers the a garbage collect list that will be destroyed once no command buffer is active. */
		void QueueDestroy(SharedPtr<Vk::Buffer>& buffer);

		/** Destroys all Vulkan resources that have been added to the garbage collect list. */
		void GarbageCollect();

		// Note: Todo: This is called from Terrain when the heightmap changes
		void UpdateInstanceAltitudes();

		ImGuiRenderer* GetUiOverlay();

		uint32_t GetWindowWidth() const;
		uint32_t GetWindowHeight() const;

	private:
		/** Adds widgets to the ImGui use interface for rendering settings. */
		void UpdateUi();

		/** Updates the cascade shadow mapping frustum splits. */
		void UpdateCascades();

	private:
		SharedPtr<JobGraph> mJobGraph;
		RenderingSettings mRenderingSettings;
		SceneInfo mSceneInfo;
		Vk::VulkanApp* mVulkanApp;
		Vk::Device* mDevice;
		Camera* mMainCamera;
		uint32_t mNextNodeId;
		std::vector<SharedPtr<Vk::Buffer>> mBuffersToFree;
	};

	Renderer& gRenderer();
}
