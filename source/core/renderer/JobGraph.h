#pragma once
#include "core/renderer/BaseJob.h"
#include "vulkan/VulkanInclude.h"

namespace Utopian
{
	/** 
	 * Each render pass is defined as a Job that can have multiple inputs and outputs.
	 * The inputs and outputs are typically render targets but can be any kind of data.
	 * JobGraph handles the order and execution of these jobs.
	 * 
	 * For example the SkydomeJob has the Depth buffer from the G-Buffer job as an input
	 * to not render the skydome infront of all objects.
	*/
	class JobGraph
	{
	public:
		enum JobIndex
		{
			GBUFFER_INDEX = 0,
			SSAO_INDEX,
			BLUR_INDEX,
			SHADOW_INDEX,
			DEFERRED_INDEX,
			//GRASS_INDEX,
			SKYBOX_INDEX,
			SUN_SHAFT_INDEX,
			//DEBUG_INDEX,
			TESSELLATION_INDEX
		};

		JobGraph(Vk::VulkanApp* vulkanApp, Terrain* terrain, Vk::Device* device, uint32_t width, uint32_t height);
		~JobGraph();

		/** Renders all jobs added to the graph. */
		void Render(const SceneInfo& sceneInfo, const RenderingSettings& renderingSettings);

		void Update();

		void EnableJob(JobIndex jobIndex, bool enabled);
	private:
		/** Adds a job to the graph. */
		void AddJob(BaseJob* job);
	private:
		std::vector<BaseJob*> mJobs;
	};
}
