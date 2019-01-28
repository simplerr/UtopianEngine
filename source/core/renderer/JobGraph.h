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
		JobGraph(Vk::Device* device, uint32_t width, uint32_t height);
		~JobGraph();

		/** Executes all jobs added to the graph. */
		void ExecuteJobs(const SceneInfo& sceneInfo, const RenderingSettings& renderingSettings);

	private:
		/** Adds a job to the graph. */
		void AddJob(BaseJob* job);
	private:
		std::vector<BaseJob*> mJobs;
	};
}
