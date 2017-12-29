#pragma once

#include <vector>
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

	void AddWater(glm::vec3 position, uint32_t numCells);

	Vulkan::RenderTarget* GetReflectionRenderTarget();
	Vulkan::RenderTarget* GetRefractionRenderTarget();

private:
	struct Water
	{
		Vulkan::StaticModel* gridModel;
		glm::vec3 position;
	};

	Vulkan::Renderer* mRenderer;
	Vulkan::ModelLoader* mModelLoader;

	Vulkan::WaterEffect mWaterEffect;
	Vulkan::RenderTarget* mReflectionRenderTarget;
	Vulkan::RenderTarget* mRefractionRenderTarget;
	Vulkan::Texture* dudvTexture;
	Vulkan::StaticModel* mGridModel;
	std::vector<Water> mWaterList;

	const uint32_t CELL_SIZE = 2000.0f;
};