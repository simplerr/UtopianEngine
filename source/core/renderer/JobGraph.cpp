#include "core/renderer/JobGraph.h"
#include "core/renderer/GBufferJob.h"
#include "core/renderer/SSAOJob.h"
#include "core/renderer/BlurJob.h"
#include "core/renderer/ShadowJob.h"
#include "core/renderer/DeferredJob.h"
#include "core/renderer/GrassJob.h"
#include "core/renderer/SkydomeJob.h"
#include "core/renderer/SunShaftJob.h"
#include "core/renderer/DebugJob.h"
#include "vulkan/handles/Device.h"
#include "vulkan/VulkanApp.h"

namespace Utopian
{
	JobGraph::JobGraph(Vk::VulkanApp* vulkanApp, Vk::Device* device, uint32_t width, uint32_t height)
	{
		GBufferJob* gbufferJob = new GBufferJob(device, width, height);
		gbufferJob->SetWaitSemaphore(vulkanApp->GetImageAvailableSemaphore());

		AddJob(gbufferJob);

		AddJob(new SSAOJob(device, width, height));
		AddJob(new BlurJob(device, width, height));
		AddJob(new ShadowJob(device, width, height));
		AddJob(new DeferredJob(device, width, height));
		//AddJob(new GrassJob(device, width, height)); // Note: Todo: Removed for syncrhonization testing
		//AddJob(new SkyboxJob(renderer, width, height));
		AddJob(new SkydomeJob(device, width, height));

		SunShaftJob* sunShaftJob = new SunShaftJob(device, width, height);
		vulkanApp->SetJobGraphWaitSemaphore(sunShaftJob->GetCompletedSemahore());
		
		AddJob(sunShaftJob);
		//AddJob(new DebugJob(device, width, height)); // Note: Todo: Removed for syncrhonization testing
	}

	JobGraph::~JobGraph()
	{
	}

	void JobGraph::ExecuteJobs(const SceneInfo& sceneInfo, const RenderingSettings& renderingSettings)
	{
		JobInput jobInput(sceneInfo, mJobs, renderingSettings);
		for (auto& job : mJobs)
		{
			job->Render(jobInput);
		}
	}

	void JobGraph::AddJob(BaseJob* job)
	{
		mJobs.push_back(job);
		job->Init(mJobs);
	}

	void JobGraph::EnableJob(JobIndex jobIndex, bool enabled)
	{
		assert(jobIndex < mJobs.size());

		mJobs[jobIndex]->SetEnabled(enabled);
	}
}