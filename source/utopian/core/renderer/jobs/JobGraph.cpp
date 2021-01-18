#include "core/renderer/ImGuiRenderer.h"
#include "core/renderer/jobs/BloomJob.h"
#include "core/renderer/jobs/BlurJob.h"
#include "core/renderer/jobs/DebugJob.h"
#include "core/renderer/jobs/DeferredJob.h"
#include "core/renderer/jobs/FXAAJob.h"
#include "core/renderer/jobs/FresnelJob.h"
#include "core/renderer/jobs/GBufferJob.h"
#include "core/renderer/jobs/GBufferTerrainJob.h"
#include "core/renderer/jobs/GeometryThicknessJob.h"
#include "core/renderer/jobs/GrassJob.h"
#include "core/renderer/jobs/Im3dJob.h"
#include "core/renderer/jobs/JobGraph.h"
#include "core/renderer/jobs/OpaqueCopyJob.h"
#include "core/renderer/jobs/PixelDebugJob.h"
#include "core/renderer/Renderer.h"
#include "core/renderer/jobs/SSAOJob.h"
#include "core/renderer/jobs/SSRJob.h"
#include "core/renderer/jobs/ShadowJob.h"
#include "core/renderer/jobs/SkyboxJob.h"
#include "core/renderer/jobs/SkydomeJob.h"
#include "core/renderer/jobs/SunShaftJob.h"
#include "core/renderer/jobs/TonemapJob.h"
#include "core/renderer/jobs/WaterJob.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/Image.h"

namespace Utopian
{
	JobGraph::JobGraph(Vk::VulkanApp* vulkanApp, Terrain* terrain, Vk::Device* device, uint32_t width, uint32_t height)
	{
		/* Create the G-buffer attachments */
		mGBuffer.positionImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R32G32B32A32_SFLOAT, "G-buffer position image");
		mGBuffer.normalImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT, "G-buffer normal (world) image");
		mGBuffer.normalViewImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R8G8B8A8_UNORM, "G-buffer normal (view) image");
		mGBuffer.albedoImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT, "G-buffer albedo image");
		mGBuffer.depthImage = std::make_shared<Vk::ImageDepth>(device, width, height, VK_FORMAT_D32_SFLOAT_S8_UINT, "G-buffer depth image");
		mGBuffer.specularImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT, "G-buffer specular image");

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
		//AddJob(new SkyboxJob(device, width, height));
		AddJob(new SkydomeJob(device, width, height));
		AddJob(new SunShaftJob(device, width, height));
		AddJob(new OpaqueCopyJob(device, width, height));
		AddJob(new GeometryThicknessJob(device, width, height));
		AddJob(new WaterJob(device, width, height));
		AddJob(new SSRJob(device, width, height));
		AddJob(new FresnelJob(device, width, height));
		AddJob(new DebugJob(device, width, height));
		AddJob(new Im3dJob(device, width, height));
		AddJob(new BloomJob(device, width, height));
		AddJob(new TonemapJob(device, width, height));
		AddJob(new PixelDebugJob(device, width, height));

		FXAAJob* fxaaJob = new FXAAJob(device, width, height);
		vulkanApp->SetWaitSubmitSemaphore(fxaaJob->GetCompletedSemahore());
		AddJob(fxaaJob);

		/* Add debug render targets */
		ImGuiRenderer* imGuiRenderer = gRenderer().GetUiOverlay();
		mDebugDescriptorSets.position = imGuiRenderer->AddImage(*mGBuffer.positionImage);
		mDebugDescriptorSets.normal = imGuiRenderer->AddImage(*mGBuffer.normalImage);
		mDebugDescriptorSets.normalView = imGuiRenderer->AddImage(*mGBuffer.normalViewImage);
		mDebugDescriptorSets.albedo = imGuiRenderer->AddImage(*mGBuffer.albedoImage);

		EnableJob(JobGraph::JobIndex::PIXEL_DEBUG_INDEX, false);
	}

	JobGraph::~JobGraph()
	{
		for(auto job : mJobs)
		{
			delete job;
		}
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

		if (ImGuiRenderer::GetMode() == UI_MODE_EDITOR)
		{
			ImGuiRenderer::BeginWindow("Render targets:", glm::vec2(300.0f, 10.0f), 400.0f);

			ImVec2 textureSize = ImVec2(256, 256);
			ImGui::BeginGroup();
			ImGui::Text("Position");
			ImGui::Image(mDebugDescriptorSets.position, textureSize);
			ImGui::EndGroup();

			ImGui::SameLine();

			ImGui::BeginGroup();
			ImGui::Text("Normal");
			ImGui::Image(mDebugDescriptorSets.normal, textureSize);
			ImGui::EndGroup();

			ImGui::BeginGroup();
			ImGui::Text("Normal view space");
			ImGui::Image(mDebugDescriptorSets.normalView, textureSize);
			ImGui::EndGroup();

			ImGui::SameLine();

			ImGui::BeginGroup();
			ImGui::Text("Albedo");
			ImGui::Image(mDebugDescriptorSets.albedo, textureSize);
			ImGui::EndGroup();

			ImGuiRenderer::EndWindow();
		}
	}

	void JobGraph::AddJob(BaseJob* job)
	{
		// Setup synchronization dependecy to the previous job
		if (mJobs.size() > 0)
		{
			SharedPtr<Vk::Semaphore>& waitSemaphore = mJobs.back()->GetCompletedSemahore();
			job->SetWaitSemaphore(waitSemaphore);
		}

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