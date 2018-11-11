#pragma once

#include <vector>
#include "vulkan/WaterEffect.h"
#include "vulkan/VulkanInclude.h"

namespace Utopian
{
	namespace Vk
	{
		class BasicRenderTarget;
	}

	class WaterRenderer
	{
	public:
		WaterRenderer(Vk::Renderer* renderer, Vk::TextureLoader* textureLoader);
		~WaterRenderer();

		void Render(Vk::Renderer* renderer, Vk::CommandBuffer* commandBuffer);
		void Update(Vk::Renderer* renderer, Camera* camera);

		void AddWater(glm::vec3 position, uint32_t numCells);

		Vk::RenderTarget* GetReflectionRenderTarget();
		Vk::RenderTarget* GetRefractionRenderTarget();

		Vk::Image* GetReflectionImage();
		Vk::Image* GetRefractionImage();

	private:
		struct Water
		{
			Vk::StaticModel* gridModel;
			glm::vec3 position;
		};

		Vk::Renderer* mRenderer;

		Vk::WaterEffect mWaterEffect;
		Vk::BasicRenderTarget* mReflectionRenderTarget;
		Vk::BasicRenderTarget* mRefractionRenderTarget;
		Vk::Texture* dudvTexture;
		Vk::StaticModel* mGridModel;
		std::vector<Water> mWaterList;

		const uint32_t CELL_SIZE = 2000.0f;
	};
}