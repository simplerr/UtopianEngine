#pragma once
#include "core/renderer/BaseJob.h"
#include "vulkan/VulkanInclude.h"
#include <glm/glm.hpp>

namespace Utopian
{
	class TessellationJob : public BaseJob
	{
	public:
		UNIFORM_BLOCK_BEGIN(ViewProjection)
			UNIFORM_PARAM(glm::mat4, projection)
			UNIFORM_PARAM(glm::mat4, view)
			UNIFORM_BLOCK_END()

		UNIFORM_BLOCK_BEGIN(SettingsBlock)
			UNIFORM_PARAM(int, tessellationFactor)
		UNIFORM_BLOCK_END()

		TessellationJob(Vk::Device* device, uint32_t width, uint32_t height);
		~TessellationJob();

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(const JobInput& jobInput) override;

		SharedPtr<Vk::Image> image;
		SharedPtr<Vk::RenderTarget> renderTarget;

	private:
		void GeneratePatches(float cellSize, int numCells);
		SharedPtr<Vk::Effect> mEffect;
		Vk::StaticModel* mQuadModel;
		ViewProjection viewProjectionBlock;
		SettingsBlock settingsBlock;
	};
}
