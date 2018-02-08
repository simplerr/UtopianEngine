#pragma once

#include <vector>
#include "vulkan/WaterEffect.h"
#include "vulkan/VulkanInclude.h"

namespace Utopian
{
	class WaterRenderer
	{
	public:
		WaterRenderer(Vk::Renderer* renderer, Vk::ModelLoader* modelLoader, Vk::TextureLoader* textureLoader);
		~WaterRenderer();

		void Render(Vk::Renderer* renderer, Vk::CommandBuffer* commandBuffer);
		void Update(Vk::Renderer* renderer, Camera* camera);

		void AddWater(glm::vec3 position, uint32_t numCells);

		Vk::RenderTarget* GetReflectionRenderTarget();
		Vk::RenderTarget* GetRefractionRenderTarget();

	private:
		struct Water
		{
			Vk::StaticModel* gridModel;
			glm::vec3 position;
		};

		Vk::Renderer* mRenderer;
		Vk::ModelLoader* mModelLoader;

		Vk::WaterEffect mWaterEffect;
		Vk::RenderTarget* mReflectionRenderTarget;
		Vk::RenderTarget* mRefractionRenderTarget;
		Vk::Texture* dudvTexture;
		Vk::StaticModel* mGridModel;
		std::vector<Water> mWaterList;

		const uint32_t CELL_SIZE = 2000.0f;
	};
}