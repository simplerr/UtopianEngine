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
#include "core/renderer/GBufferTerrainJob.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/Image.h"
#include "vulkan/VulkanApp.h"

namespace Utopian
{
	JobGraph::JobGraph(Vk::VulkanApp* vulkanApp, const SharedPtr<Terrain>& terrain, Vk::Device* device, uint32_t width, uint32_t height)
	{
		/* Create the G-buffer attachments */
		mGBuffer.positionImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R32G32B32A32_SFLOAT);
		mGBuffer.normalImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT);
		mGBuffer.normalViewImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R8G8B8A8_UNORM);
		mGBuffer.albedoImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R8G8B8A8_UNORM);
		mGBuffer.depthImage = std::make_shared<Vk::ImageDepth>(device, width, height, VK_FORMAT_D32_SFLOAT_S8_UINT);

		/* Add jobs */
		GBufferTerrainJob* gbufferTerrainJob = new GBufferTerrainJob(device, terrain, width, height);
		gbufferTerrainJob->SetWaitSemaphore(vulkanApp->GetImageAvailableSemaphore());
		AddJob(gbufferTerrainJob);

		AddJob(new GBufferJob(device, width, height));
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

	void JobGraph::Render(const SceneInfo& sceneInfo, const RenderingSettings& renderingSettings)
	{
		JobInput jobInput(sceneInfo, mJobs, renderingSettings);
		for (auto& job : mJobs)
		{
			job->Render(jobInput);
		}
	}

	void JobGraph::Update()
	{
		for (auto& job : mJobs)
		{
			job->Update();
		}
	}

	void JobGraph::AddJob(BaseJob* job)
	{
		mJobs.push_back(job);
		job->Init(mJobs, mGBuffer);
	}

	void JobGraph::EnableJob(JobIndex jobIndex, bool enabled)
	{
		assert(jobIndex < mJobs.size());

		mJobs[jobIndex]->SetEnabled(enabled);
	}

	const GBuffer& JobGraph::GetGBuffer() const
	{
		return mGBuffer;
	}
}