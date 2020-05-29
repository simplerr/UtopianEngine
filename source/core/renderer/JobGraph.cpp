#include "ImGuiRenderer.h"
#include "core/renderer/BloomJob.h"
#include "core/renderer/BlurJob.h"
#include "core/renderer/DebugJob.h"
#include "core/renderer/DeferredJob.h"
#include "core/renderer/FXAAJob.h"
#include "core/renderer/FresnelJob.h"
#include "core/renderer/GBufferJob.h"
#include "core/renderer/GBufferTerrainJob.h"
#include "core/renderer/GeometryThicknessJob.h"
#include "core/renderer/GrassJob.h"
#include "core/renderer/Im3dJob.h"
#include "core/renderer/JobGraph.h"
#include "core/renderer/OpaqueCopyJob.h"
#include "core/renderer/PixelDebugJob.h"
#include "core/renderer/Renderer.h"
#include "core/renderer/SSAOJob.h"
#include "core/renderer/SSRJob.h"
#include "core/renderer/ShadowJob.h"
#include "core/renderer/SkyboxJob.h"
#include "core/renderer/SkydomeJob.h"
#include "core/renderer/SunShaftJob.h"
#include "core/renderer/TonemapJob.h"
#include "core/renderer/WaterJob.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/Image.h"

namespace Utopian
{
	JobGraph::JobGraph(Vk::VulkanApp* vulkanApp, const SharedPtr<Terrain>& terrain, Vk::Device* device, uint32_t width, uint32_t height)
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
		vulkanApp->SetJobGraphWaitSemaphore(fxaaJob->GetCompletedSemahore());
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
		// Todo: Delete jobs...
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

		// Display Actor creation list
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