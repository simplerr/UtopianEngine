#include "SceneRenderer.h"
#include "WaterRenderer.h"
#include "vulkan/ScreenGui.h"
#include "vulkan/Renderer.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/ScreenGui.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/handles/CommandBuffer.h"
#include "ecs/systems/RenderSystem.h"
#include "Terrain.h"
#include "Camera.h"

SceneRenderer::SceneRenderer(Vulkan::Renderer* renderer, Vulkan::Camera* camera)
{
    mRenderer = renderer;
    mCamera = camera;

    mTerrain = new Terrain(mRenderer, mCamera);

    mWaterRenderer = new WaterRenderer(mRenderer, renderer->mModelLoader, renderer->mTextureLoader);
    mWaterRenderer->AddWater(glm::vec3(123000.0f, 0.0f, 106000.0f), 20);
    mWaterRenderer->AddWater(glm::vec3(103000.0f, 0.0f, 96000.0f), 20);

    mScreenGui = new Vulkan::ScreenGui(mRenderer);
    mScreenGui->AddQuad(mRenderer->GetWindowWidth() - 2*350 - 50, mRenderer->GetWindowHeight() - 350, 300, 300, mWaterRenderer->GetReflectionRenderTarget()->GetImage(), mWaterRenderer->GetReflectionRenderTarget()->GetSampler());
    mScreenGui->AddQuad(mRenderer->GetWindowWidth() - 350, mRenderer->GetWindowHeight() - 350, 300, 300, mWaterRenderer->GetRefractionRenderTarget()->GetImage(), mWaterRenderer->GetRefractionRenderTarget()->GetSampler());

	mCommandBuffer = mRenderer->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);
	mTerrainCommandBuffer = mRenderer->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);
}

SceneRenderer::~SceneRenderer()
{
    delete mTerrain;
    delete mScreenGui;
    delete mWaterRenderer;
}

void SceneRenderer::Update()
{
    mTerrain->Update();
    mWaterRenderer->Update(mRenderer, mCamera);
    // GrassRenderer
}

void SceneRenderer::Render()
{
    RenderScene();

    RenderOffscreen();

	mCommandBuffer->Begin(mRenderer->GetRenderPass(), mRenderer->GetCurrentFrameBuffer());
	mCommandBuffer->CmdSetViewPort(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());
	mCommandBuffer->CmdSetScissor(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());

    mScreenGui->Render(mRenderer, mCommandBuffer);
    mWaterRenderer->Render(mRenderer, mCommandBuffer);
	
	mCommandBuffer->End();

    // Render RenderSystem

    // Handles the reflections

}

void SceneRenderer::RenderScene()
{
    // Render terrain and all models from the RenderSystem
    mTerrainCommandBuffer->Begin(mRenderer->GetRenderPass(), mRenderer->GetCurrentFrameBuffer());
    mTerrainCommandBuffer->CmdSetViewPort(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());
    mTerrainCommandBuffer->CmdSetScissor(mRenderer->GetWindowWidth(), mRenderer->GetWindowHeight());

    mTerrain->Render(mTerrainCommandBuffer);
    mTerrainCommandBuffer->End();

    mRenderSystem->Render();
}

void SceneRenderer::RenderOffscreen()
{
    glm::vec3 cameraPos = mCamera->GetPosition();

    // Reflection renderpass
    mCamera->SetPosition(mCamera->GetPosition() - glm::vec3(0, mCamera->GetPosition().y *  2, 0)); // NOTE: Water is hardcoded to be at y = 0
    mCamera->SetOrientation(mCamera->GetYaw(), -mCamera->GetPitch());
    mTerrain->SetClippingPlane(glm::vec4(0, -1, 0, 0));
    mTerrain->UpdateUniformBuffer();

    mWaterRenderer->GetReflectionRenderTarget()->Begin();	

    mTerrain->Render(mWaterRenderer->GetReflectionRenderTarget()->GetCommandBuffer());
    // RendeScene() instead

    mWaterRenderer->GetReflectionRenderTarget()->End(mRenderer->GetQueue());

    // Refraction renderpass
    mTerrain->SetClippingPlane(glm::vec4(0, 1, 0, 0));
    mCamera->SetPosition(cameraPos);
    mCamera->SetOrientation(mCamera->GetYaw(), -mCamera->GetPitch());
    mTerrain->UpdateUniformBuffer();

    mWaterRenderer->GetRefractionRenderTarget()->Begin();	

    mTerrain->Render(mWaterRenderer->GetRefractionRenderTarget()->GetCommandBuffer());
    // RendeScene() instead

    mWaterRenderer->GetRefractionRenderTarget()->End(mRenderer->GetQueue());

    mTerrain->SetClippingPlane(glm::vec4(0, 1, 0, 1500000));
    mTerrain->UpdateUniformBuffer();
}

Terrain* SceneRenderer::GetTerrain()
{
    return mTerrain;
}

void SceneRenderer::SetRenderSystem(ECS::RenderSystem* renderSystem)
{
    mRenderSystem = renderSystem;
}