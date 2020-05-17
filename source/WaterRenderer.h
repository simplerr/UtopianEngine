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
		WaterRenderer(Vk::TextureLoader* textureLoader, Vk::VulkanApp* vulkanApp);
		~WaterRenderer();

		void Render(Vk::CommandBuffer* commandBuffer);
		void Update(Camera* camera);

		void AddWater(glm::vec3 position, uint32_t numCells);

		Vk::RenderTarget* GetReflectionRenderTarget();
		Vk::RenderTarget* GetRefractionRenderTarget();

	private:
		struct Water
		{
			Vk::StaticModel* gridModel;
			glm::vec3 position;
		};

		Vk::WaterEffect mWaterEffect;
		Vk::BasicRenderTarget* mReflectionRenderTarget;
		Vk::BasicRenderTarget* mRefractionRenderTarget;
		SharedPtr<Vk::Texture> dudvTexture;
		Vk::StaticModel* mGridModel;
		std::vector<Water> mWaterList;

		const uint32_t CELL_SIZE = 2000.0f;
	};
}