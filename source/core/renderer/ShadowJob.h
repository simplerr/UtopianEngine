#pragma once

#include "core/renderer/BaseJob.h"

namespace Utopian
{
	class ShadowJob : public BaseJob
	{
	public:

		UNIFORM_BLOCK_BEGIN(CascadeTransforms)
			UNIFORM_PARAM(glm::mat4, viewProjection[SHADOW_MAP_CASCADE_COUNT])
		UNIFORM_BLOCK_END()

		struct CascadePushConst
		{
			CascadePushConst(glm::mat4 _world, uint32_t _cascadeIndex) {

				world = _world;
				// Note: This needs to be done to have the physical world match the rendered world.
				// See https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/ for more information.
				world[3][0] = -world[3][0];
				world[3][1] = -world[3][1];
				world[3][2] = -world[3][2];

				cascadeIndex = _cascadeIndex;
			}

			glm::mat4 world;
			uint32_t cascadeIndex;
		};

		ShadowJob(Vk::Device* device, uint32_t width, uint32_t height);
		~ShadowJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;

		SharedPtr<Vk::RenderTarget> renderTarget;
		SharedPtr<Vk::Image> depthColorImage;
		SharedPtr<Vk::Image> depthImage;

		SharedPtr<Vk::Effect> effect;
		SharedPtr<Vk::Effect> effectInstanced;
		CascadeTransforms cascadeTransforms;
	private:
		const uint32_t SHADOWMAP_DIMENSION = 4096;
		std::vector<SharedPtr<Vk::FrameBuffers>> mFrameBuffers;
	};
}
