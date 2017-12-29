#pragma once

#include "vulkan/WaterEffect.h"

namespace Vulkan
{
	class CommandBuffer;
	class RenderTarget;
	class Texture;
	class TextureLoader;
	class StaticModel;
	class Renderer;
	class ModelLoader;
	class Camera;
}

class WaterRenderer
{
public:
	WaterRenderer(Vulkan::Renderer* renderer, Vulkan::ModelLoader* modelLoader, Vulkan::TextureLoader* textureLoader);
	~WaterRenderer();

	void Render(Vulkan::Renderer* renderer, Vulkan::CommandBuffer* commandBuffer);
	void Update(Vulkan::Renderer* renderer, Vulkan::Camera* camera);

	Vulkan::RenderTarget* GetReflectionRenderTarget();
	Vulkan::RenderTarget* GetRefractionRenderTarget();

private:
	Vulkan::WaterEffect mWaterEffect;
	Vulkan::RenderTarget* mReflectionRenderTarget;
	Vulkan::RenderTarget* mRefractionRenderTarget;
	Vulkan::Texture* dudvTexture;
	Vulkan::StaticModel* mGridModel;
};