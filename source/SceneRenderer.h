#pragma once

namespace ECS
{
    class RenderSystem;
}

namespace Vulkan
{
    class Camera;
    class Renderer;
    class ScreenGui;
	class CommandBuffer;
}

class Terrain;
class WaterRenderer;

class SceneRenderer
{
public:
    SceneRenderer(Vulkan::Renderer* renderer, Vulkan::Camera* camera);
    ~SceneRenderer();

    void Update();
    void Render();

    void SetRenderSystem(ECS::RenderSystem* renderSystem);
    Terrain* GetTerrain();
private:
    void RenderScene();
    void RenderOffscreen();

    Vulkan::Renderer* mRenderer;
    Vulkan::Camera* mCamera;
    Vulkan::ScreenGui* mScreenGui;
    Terrain* mTerrain;
    WaterRenderer* mWaterRenderer;
    ECS::RenderSystem* mRenderSystem;

	Vulkan::CommandBuffer* mCommandBuffer;
	Vulkan::CommandBuffer* mTerrainCommandBuffer;
    // BillboardSystem* mBillboardSystem;
    // ParticleSystem* mParticleSystem;
};