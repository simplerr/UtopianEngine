#include <core/renderer/RendererUtility.h>
#include <glm/gtc/matrix_transform.hpp>
#include "core/renderer/Renderer.h"
#include "core/renderer/Renderable.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/Vertex.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/Image.h"
#include "core/Camera.h"
#include "vulkan/Mesh.h"
#include "vulkan/StaticModel.h"
#include "core/renderer/Light.h"
#include "core/renderer/ScreenQuadRenderer.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/BasicRenderTarget.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/TextureLoader.h"
#include "core/renderer/ImGuiRenderer.h"
#include "core/renderer/Im3dRenderer.h"
#include "core/Terrain.h"
#include "core/AssetLoader.h"
#include "core/renderer/jobs/GBufferJob.h"
#include "core/renderer/jobs/SSAOJob.h"
#include "core/renderer/jobs/BlurJob.h"
#include "core/renderer/jobs/ShadowJob.h"
#include "core/renderer/jobs/DeferredJob.h"
#include "core/renderer/jobs/GrassJob.h"
#include "core/renderer/jobs/SkydomeJob.h"
#include "core/renderer/jobs/SunShaftJob.h"
#include "core/renderer/jobs/DebugJob.h"
#include "core/renderer/InstancingManager.h"
#include "core/ScriptExports.h"
#include "core/renderer/ScreenQuadRenderer.h"
#include "core/Log.h"
#include "utility/math/Helpers.h"
#include "core/Input.h"

#include <fstream>
#include <glm/gtx/string_cast.hpp>

namespace Utopian
{
	Renderer& gRenderer()
	{
		return Renderer::Instance();
	}

	Renderer::Renderer(Vk::VulkanApp* vulkanApp)
	{
		UTO_LOG("Initializing Renderer");

		mNextNodeId = 0;
		mMainCamera = nullptr;
		mVulkanApp = vulkanApp;
		mDevice = vulkanApp->GetDevice();

		mSceneInfo.directionalLight = nullptr;

		// Todo: Figure out where these belong
		mIm3dRenderer = std::make_shared<Im3dRenderer>(mVulkanApp, glm::vec2(mVulkanApp->GetWindowWidth(), mVulkanApp->GetWindowHeight()));

		mSceneInfo.sharedVariables.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	}

	Renderer::~Renderer()
	{
		// Cannot rely on instance group being destroyed when going out of scope since that happens
		// after the call to GarbageCollect()
		ClearInstanceGroups();
	}

	void Renderer::PostWorldInit()
	{
		mInstancingManager = std::make_shared<InstancingManager>(this);

		if (mRenderingSettings.terrainEnabled)
			mSceneInfo.terrain = std::make_shared<Terrain>(mDevice);
		else
			mSceneInfo.terrain = nullptr;

		ScriptExports::SetTerrain(GetTerrain());

		mJobGraph = std::make_shared<JobGraph>(mVulkanApp, GetTerrain(), mDevice, mVulkanApp->GetWindowWidth(), mVulkanApp->GetWindowHeight());

		LoadInstancesFromFile("data/instances.txt");
		BuildAllInstances();
	}

	void Renderer::Update()
	{
		UpdateCascades();

		if (mSceneInfo.terrain != nullptr)
			mSceneInfo.terrain->Update();

		mMainCamera->UpdateFrustum();
		mJobGraph->Update();

		//BuildAllInstances();

		UpdateUi();
	}

	void Renderer::UpdateUi()
	{
		// Draw UI overlay for rendering settings
		// It's expected that each rendering node might have it's own settings that can be configured
		ImGuiRenderer::BeginWindow("Rendering settings", glm::vec2(10, 150), 300.0f);

		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

		if (ImGui::CollapsingHeader("Features", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Checkbox("Shadows", &mRenderingSettings.shadowsEnabled);
			ImGui::Checkbox("Normal mapping", &mRenderingSettings.normalMapping);
			ImGui::Checkbox("SSAO", &mRenderingSettings.ssaoEnabled);
			ImGui::Checkbox("SSR", &mRenderingSettings.ssrEnabled);
			ImGui::Checkbox("Skybox reflections", &mRenderingSettings.skyboxReflections);
			ImGui::Checkbox("Water", &mRenderingSettings.waterEnabled);
			ImGui::Checkbox("God rays", &mRenderingSettings.godRaysEnabled);
			ImGui::Checkbox("FXAA", &mRenderingSettings.fxaaEnabled);
			ImGui::Checkbox("FXAA debug", &mRenderingSettings.fxaaDebug);
			ImGui::Checkbox("Cascade color debug", &mRenderingSettings.cascadeColorDebug);
			ImGui::Checkbox("Terrain wireframe", &mRenderingSettings.terrainWireframe);
			ImGui::Checkbox("Wind enabled", &mRenderingSettings.windEnabled);

			static bool debugQuads = true;
			ImGui::Checkbox("Debug quads", &debugQuads);
			gScreenQuadUi().SetVisible(0, debugQuads);
		}

		if (ImGui::CollapsingHeader("Fog settings"))
		{
			ImGui::ColorEdit4("Fog color", &mRenderingSettings.fogColor.x);
			ImGui::SliderFloat("Fog start", &mRenderingSettings.fogStart, 0.0f, 800.0f);
			ImGui::SliderFloat("Fog distance", &mRenderingSettings.fogDistance, 0.0f, 800.0f);
		}

		//ImGui::SliderFloat("SSAO radius", &mRenderingSettings.ssaoRadius, 0.0f, 20.0f);
		//ImGui::SliderFloat("SSAO bias", &mRenderingSettings.ssaoBias, 0.0f, 10.0f);
		//ImGui::SliderInt("SSAO blur radius", &mRenderingSettings.blurRadius, 1, 20);
		//ImGui::SliderInt("Block view distance", &mRenderingSettings.blockViewDistance, 1, 10);
		//ImGui::SliderFloat("Grass view distance", &mRenderingSettings.grassViewDistance, 0.0f, 10000.0f);

		if (ImGui::CollapsingHeader("Mixed settings"))
		{
			ImGui::SliderFloat("FXAA threshold", &mRenderingSettings.fxaaThreshold, 0.0f, 1.5f);
			ImGui::SliderInt("Shadow sample size", &mRenderingSettings.shadowSampleSize, 0, 10);
			ImGui::SliderFloat("Cascade split lambda", &mRenderingSettings.cascadeSplitLambda, 0.0f, 1.0f);
			ImGui::SliderFloat("Sun inclination", &mRenderingSettings.sunInclination, -90.0f, 90.0f);
			//ImGui::SliderFloat("Sun azimuth", &mRenderingSettings.sunAzimuth, -180.0f, 180.0f);
			ImGui::SliderFloat("Sun speed", &mRenderingSettings.sunSpeed, 0.0f, 10.0f);
			ImGui::SliderFloat("Exposure", &mRenderingSettings.exposure, 0.0f, 5.0f);
			ImGui::Combo("Tonemapping", &mRenderingSettings.tonemapping, "Reinhard\0Uncharted 2\0Exposure\0None\0");
			ImGui::SliderFloat("Bloom threshold", &mRenderingSettings.bloomThreshold, 0.5f, 10.0f);
			ImGui::SliderFloat("Wind strength", &mRenderingSettings.windStrength, 0.0f, 25.0f);
			ImGui::SliderFloat("Wind frequency", &mRenderingSettings.windFrequency, 0.0f, 30000.0f);
		}

		if (ImGui::CollapsingHeader("Terrain settings"))
		{
			ImGui::SliderFloat("Tessellation factor", &mRenderingSettings.tessellationFactor, 0.0f, 5.0f);

			float amplitudeScaling = mSceneInfo.terrain->GetAmplitudeScaling();
			if (ImGui::SliderFloat("Terrain amplitude", &amplitudeScaling, 0.4f, 100))
			{
				mSceneInfo.terrain->SetAmplitudeScaling(amplitudeScaling);
				//UpdateInstanceAltitudes(); Can't do this because the buffer is in use by a command buffer
			}

			ImGui::SliderFloat("Terrain texture scaling", &mRenderingSettings.terrainTextureScaling, 1.0f, 600.0f);
			ImGui::SliderFloat("Terrain bumpmap amplitude", &mRenderingSettings.terrainBumpmapAmplitude, 0.0078f, 0.4);
		}

		if (ImGui::CollapsingHeader("Water settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderFloat("Water level", &mRenderingSettings.waterLevel, -80.0f, 80.0f);
			ImGui::ColorEdit3("Water color", &mRenderingSettings.waterColor.x);
			ImGui::ColorEdit3("Foam color", &mRenderingSettings.foamColor.x);
			ImGui::SliderFloat("Wave speed", &mRenderingSettings.waveSpeed, 0.1f, 10.0f);
			ImGui::SliderFloat("Foam speed", &mRenderingSettings.foamSpeed, 0.1f, 10.0f);
			ImGui::SliderFloat("Distortion strength", &mRenderingSettings.waterDistortionStrength, 0.005f, 0.1f);
			ImGui::SliderFloat("Shoreline depth", &mRenderingSettings.shorelineDepth, 0.0f, 8.0f);
			ImGui::SliderFloat("Wave frequency", &mRenderingSettings.waveFrequency, 0.0f, 1000.0f);
			ImGui::SliderFloat("Water specularity", &mRenderingSettings.waterSpecularity, 1.0f, 1024.0f);
			ImGui::SliderFloat("Water transparency", &mRenderingSettings.waterTransparency, 0.0f, 1.0f);
			ImGui::SliderFloat("Underwater view distance", &mRenderingSettings.underwaterViewDistance, 0.0f, 40.0f);
		}

		mJobGraph->EnableJob(JobGraph::JobIndex::SSAO_INDEX, mRenderingSettings.ssaoEnabled);
		mJobGraph->EnableJob(JobGraph::JobIndex::BLUR_INDEX, mRenderingSettings.ssaoEnabled);
		mJobGraph->EnableJob(JobGraph::JobIndex::SSR_INDEX, mRenderingSettings.ssrEnabled);
		mJobGraph->EnableJob(JobGraph::JobIndex::WATER_INDEX, mRenderingSettings.waterEnabled);
		mJobGraph->EnableJob(JobGraph::JobIndex::SHADOW_INDEX, mRenderingSettings.shadowsEnabled);
		mJobGraph->EnableJob(JobGraph::JobIndex::SUN_SHAFT_INDEX, mRenderingSettings.godRaysEnabled);

		ImGuiRenderer::EndWindow();

		ImGuiRenderer::BeginWindow("Utopian Engine (v0.2)", glm::vec2(10, 10), 350.0f);

		glm::vec3 pos = mMainCamera->GetPosition();
		glm::vec3 dir = mMainCamera->GetDirection();

		ImGuiRenderer::TextV(std::string("Vulkan " + GetDevice()->GetVulkanVersion().version).c_str());
		ImGuiRenderer::TextV("Time: %.2f", Timer::Instance().GetTime());
		ImGuiRenderer::TextV("FPS: %u", Timer::Instance().GetFPS());
		ImGuiRenderer::TextV("Camera pos = (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
		ImGuiRenderer::TextV("Camera dir = (%.2f, %.2f, %.2f)", dir.x, dir.y, dir.z);
		ImGuiRenderer::TextV("Models: %u, Lights: %u", mSceneInfo.renderables.size(), mSceneInfo.lights.size());

		if (ImGui::Button("Dump memory statistics"))
		{
			mDevice->DumpMemoryStats("memory-statistics.json");
		}

		ImGuiRenderer::EndWindow();
	}

	/*
		Calculate frustum split depths and matrices for the shadow map cascades
		Based on https://johanmedestrom.wordpress.com/2016/03/18/opengl-cascaded-shadow-maps/
		From Sascha Willems example demos.
	*/
	void Renderer::UpdateCascades()
	{
		float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];

		float nearClip = mMainCamera->GetNearPlane();
		float farClip = mMainCamera->GetFarPlane();
		float clipRange = farClip - nearClip;

		float minZ = nearClip;
		float maxZ = nearClip + clipRange;

		float range = maxZ - minZ;
		float ratio = maxZ / minZ;

		// Calculate split depths based on view camera furstum
		// Based on method presentd in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			float p = (i + 1) / static_cast<float>(SHADOW_MAP_CASCADE_COUNT);
			float log = minZ * std::pow(ratio, p);
			float uniform = minZ + range * p;
			float d = mRenderingSettings.cascadeSplitLambda * (log - uniform) + uniform;
			cascadeSplits[i] = (d - nearClip) / clipRange;
		}

		// Calculate orthographic projection matrix for each cascade
		float lastSplitDist = 0.0;
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			float splitDist = cascadeSplits[i];

			glm::vec3 frustumCorners[8] = {
				glm::vec3(-1.0f,  1.0f, -1.0f),
				glm::vec3(1.0f,  1.0f, -1.0f),
				glm::vec3(1.0f, -1.0f, -1.0f),
				glm::vec3(-1.0f, -1.0f, -1.0f),
				glm::vec3(-1.0f,  1.0f,  1.0f),
				glm::vec3(1.0f,  1.0f,  1.0f),
				glm::vec3(1.0f, -1.0f,  1.0f),
				glm::vec3(-1.0f, -1.0f,  1.0f),
			};

			// Project frustum corners into world space
			glm::mat4 invCam = glm::inverse(mMainCamera->GetProjection() * mMainCamera->GetView());
			for (uint32_t i = 0; i < 8; i++) {
				glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
				frustumCorners[i] = invCorner / invCorner.w;
			}

			for (uint32_t i = 0; i < 4; i++) {
				glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
				frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
				frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
			}

			// Get frustum center
			glm::vec3 frustumCenter = glm::vec3(0.0f);
			for (uint32_t i = 0; i < 8; i++) {
				frustumCenter += frustumCorners[i];
			}
			frustumCenter /= 8.0f;

			float radius = 0.0f;
			for (uint32_t i = 0; i < 8; i++) {
				float distance = glm::length(frustumCorners[i] - frustumCenter);
				radius = glm::max(radius, distance);
			}
			radius = std::ceil(radius * 16.0f) / 16.0f;

			glm::vec3 maxExtents = glm::vec3(radius);
			glm::vec3 minExtents = -maxExtents;

			// Todo: Note: vec3(-1, 1, -1) is needed to make the shadows match phong shading
			glm::vec3 lightDir = glm::vec3(-1, 1, -1) * glm::normalize(mSceneInfo.directionalLight->GetDirection());
			glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));

			// glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);
			// Note: from Saschas examples the zNear was 0.0f, unclear why I need to set it to -(maxExtents.z - minExtents.z).
			glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, -(maxExtents.z - minExtents.z), maxExtents.z - minExtents.z);

			// Store split distance and matrix in cascade
			mSceneInfo.cascades[i].splitDepth = (mMainCamera->GetNearPlane() + splitDist * clipRange) * -1.0f;
			mSceneInfo.cascades[i].viewProjMatrix = lightOrthoMatrix * lightViewMatrix;

			lastSplitDist = cascadeSplits[i];
		}
	}

	void Renderer::Render()
	{
		// Note: This had to be done here due to the different periodicity of Update() and Render().
		// This should be corrected.
		mIm3dRenderer->UploadVertexData();

		// Deferred pipeline
		if (mRenderingSettings.deferredPipeline == true)
		{
			mSceneInfo.viewMatrix = mMainCamera->GetView();
			mSceneInfo.projectionMatrix = mMainCamera->GetProjection();
			mSceneInfo.eyePos = mMainCamera->GetPosition();
			mSceneInfo.im3dVertices = mIm3dRenderer->GetVertexBuffer();

			mSceneInfo.sharedVariables.data.viewMatrix = mMainCamera->GetView();
			mSceneInfo.sharedVariables.data.projectionMatrix = mMainCamera->GetProjection();
			mSceneInfo.sharedVariables.data.inverseProjectionMatrix = glm::inverse(glm::mat3(mMainCamera->GetProjection()));
			mSceneInfo.sharedVariables.data.eyePos = glm::vec4(mMainCamera->GetPosition(), 1.0f);
			mSceneInfo.sharedVariables.data.mouseUV = gInput().GetMousePosition();
			mSceneInfo.sharedVariables.data.time = gTimer().GetTime();
			mSceneInfo.sharedVariables.data.viewportSize = glm::vec2(mVulkanApp->GetWindowWidth(), mVulkanApp->GetWindowHeight());
			mSceneInfo.sharedVariables.UpdateMemory();

			mJobGraph->Render(mSceneInfo, mRenderingSettings);
		}

		gScreenQuadUi().Render(mVulkanApp);
	}

	void Renderer::NewUiFrame()
	{
		mIm3dRenderer->NewFrame();
	}

	void Renderer::EndUiFrame()
	{
		mIm3dRenderer->EndFrame();
	}

	void Renderer::AddRenderable(Renderable* renderable)
	{
		renderable->SetId(mNextNodeId++);
		mSceneInfo.renderables.push_back(renderable);
	}

	void Renderer::AddLight(Light* light)
	{
		if (light->GetType() == 0) { // Directional
			if (mSceneInfo.directionalLight == nullptr)
				mSceneInfo.directionalLight = light;
		}

		light->SetId(mNextNodeId++);
		mSceneInfo.lights.push_back(light);
	}

	void Renderer::AddCamera(Camera* camera)
	{
		camera->SetId(mNextNodeId++);
		mSceneInfo.cameras.push_back(camera);
	}

	void Renderer::RemoveRenderable(Renderable* renderable)
	{
		for (auto iter = mSceneInfo.renderables.begin(); iter != mSceneInfo.renderables.end(); iter++)
		{
			// Note: No need to free memory here since that will happen when the SharedPtr is removed from the CRenderable
			if ((*iter)->GetId() == renderable->GetId())
			{
				mSceneInfo.renderables.erase(iter);
				break;
			}
		}
	}

	void Renderer::RemoveLight(Light* light)
	{
		for (auto iter = mSceneInfo.lights.begin(); iter != mSceneInfo.lights.end(); iter++)
		{
			if ((*iter)->GetId() == light->GetId())
			{
				mSceneInfo.lights.erase(iter);
				break;
			}
		}
	}

	void Renderer::RemoveCamera(Camera* camera)
	{
		for (auto iter = mSceneInfo.cameras.begin(); iter != mSceneInfo.cameras.end(); iter++)
		{
			if ((*iter)->GetId() == camera->GetId())
			{
				mSceneInfo.cameras.erase(iter);
				break;
			}
		}
	}

	void Renderer::UpdateInstanceAltitudes()
	{
		mInstancingManager->UpdateInstanceAltitudes();
	}

	void Renderer::AddInstancedAsset(uint32_t assetId, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, bool animated, bool castShadow)
	{
		mInstancingManager->AddInstancedAsset(assetId, position, rotation, scale, animated, castShadow);
	}

	void Renderer::RemoveInstancesWithinRadius(uint32_t assetId, glm::vec3 position, float radius)
	{
		mInstancingManager->RemoveInstancesWithinRadius(assetId, position, radius);
	}

	void Renderer::ClearInstanceGroups()
	{
		mInstancingManager->ClearInstanceGroups();
	}

	void Renderer::SaveInstancesToFile(const std::string& filename)
	{
		mInstancingManager->SaveInstancesToFile(filename);
	}

	void Renderer::LoadInstancesFromFile(const std::string& filename)
	{
		mInstancingManager->LoadInstancesFromFile(filename);
	}

	void Renderer::BuildAllInstances()
	{
		mInstancingManager->BuildAllInstances();
	}

	void Renderer::SetMainCamera(Camera* camera)
	{
		mMainCamera = camera;
	}

	void Renderer::SetUiOverlay(ImGuiRenderer* imguiRenderer)
	{
		mImGuiRenderer = imguiRenderer;
	}

	Terrain* Renderer::GetTerrain() const
	{
		return mSceneInfo.terrain.get();
	}

	SceneInfo* Renderer::GetSceneInfo()
	{
		return &mSceneInfo;
	}

	const RenderingSettings& Renderer::GetRenderingSettings() const
	{
		return mRenderingSettings;
	}

	void Renderer::SetRenderingSettings(const RenderingSettings& renderingSettings)
	{
		mRenderingSettings = renderingSettings;
	}

	const SharedShaderVariables& Renderer::GetSharedShaderVariables() const
	{
		return mSceneInfo.sharedVariables;
	}

	Camera* Renderer::GetMainCamera() const
	{
		return mMainCamera;
	}

	Vk::Device* Renderer::GetDevice() const
	{
		return mDevice;
	}

	ImGuiRenderer* Renderer::GetUiOverlay()
	{
		return mImGuiRenderer;
	}

	uint32_t Renderer::GetWindowWidth() const
	{
		return mVulkanApp->GetWindowWidth();
	}

	uint32_t Renderer::GetWindowHeight() const
	{
		return mVulkanApp->GetWindowHeight();
	}
}