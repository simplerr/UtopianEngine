#pragma once
#include "core/renderer/CommonBuffers.h"
#include "core/renderer/jobs/BaseJob.h"
#include "core/renderer/RenderSettings.h"
#include "core/renderer/jobs/JobGraph.h"
#include "core/Object.h"
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "utility/Module.h"
#include "utility/Common.h"
#include "utility/Timer.h"
#include <vector>

namespace Utopian
{
	class ImGuiRenderer;
	class Im3dRenderer;
	class InstancingManager;

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

		/** New frame of ImGui and Im3d */
		void NewUiFrame();

		/** End if frame for ImGui and Im3d */
		void EndUiFrame();

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
		Terrain* GetTerrain() const;

		/** Returns the scene info. */
		SceneInfo* GetSceneInfo();

		/** Returns the configured rendering settings. */
		const RenderingSettings& GetRenderingSettings() const;

		const SharedShaderVariables& GetSharedShaderVariables() const;

		/** Instancing experimentation. */
		void AddInstancedAsset(uint32_t assetId, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, bool animated = false, bool castShadow = false);
		void RemoveInstancesWithinRadius(uint32_t assetId, glm::vec3 position, float radius);
		void BuildAllInstances();
		void ClearInstanceGroups();
		void SaveInstancesToFile(const std::string& filename);
		void LoadInstancesFromFile(const std::string& filename);

		/** Garbage collects ImGui textures. */
		void GarbageCollectUiTextures();

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
		SharedPtr<InstancingManager> mInstancingManager;
		RenderingSettings mRenderingSettings;
		SceneInfo mSceneInfo;
		Vk::VulkanApp* mVulkanApp;
		Vk::Device* mDevice;
		Camera* mMainCamera;
		uint32_t mNextNodeId;

		// Where does this belong?
	public:
		SharedPtr<Im3dRenderer> mIm3dRenderer = nullptr;
		SharedPtr<ImGuiRenderer> mImGuiRenderer = nullptr;
	};

	Renderer& gRenderer();
}
