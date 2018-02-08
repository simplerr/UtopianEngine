#pragma once

#include <vector>
#include "vulkan/WaterEffect.h"

namespace Utopian::Vk
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
	WaterRenderer(Utopian::Vk::Renderer* renderer, Utopian::Vk::ModelLoader* modelLoader, Utopian::Vk::TextureLoader* textureLoader);
	~WaterRenderer();

	void Render(Utopian::Vk::Renderer* renderer, Utopian::Vk::CommandBuffer* commandBuffer);
	void Update(Utopian::Vk::Renderer* renderer, Utopian::Vk::Camera* camera);

	void AddWater(glm::vec3 position, uint32_t numCells);

	Utopian::Vk::RenderTarget* GetReflectionRenderTarget();
	Utopian::Vk::RenderTarget* GetRefractionRenderTarget();

private:
	struct Water
	{
		Utopian::Vk::StaticModel* gridModel;
		glm::vec3 position;
	};

	Utopian::Vk::Renderer* mRenderer;
	Utopian::Vk::ModelLoader* mModelLoader;

	Utopian::Vk::WaterEffect mWaterEffect;
	Utopian::Vk::RenderTarget* mReflectionRenderTarget;
	Utopian::Vk::RenderTarget* mRefractionRenderTarget;
	Utopian::Vk::Texture* dudvTexture;
	Utopian::Vk::StaticModel* mGridModel;
	std::vector<Water> mWaterList;

	const uint32_t CELL_SIZE = 2000.0f;
};