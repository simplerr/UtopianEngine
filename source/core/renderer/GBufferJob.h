#pragma once

#include "core/renderer/BaseJob.h"
#include "vulkan/GBufferEffect.h"

namespace Utopian
{
	#define NUM_MAX_SPHERES 64
	struct SphereInfo
	{
		glm::vec3 position;
		float radius;
	};

	class SphereUniformBuffer : public Utopian::Vk::ShaderBuffer
	{
	public:
		virtual void UpdateMemory()
		{
			uint8_t* mapped;
			uint32_t dataOffset = 0;
			uint32_t dataSize = sizeof(constants);
			mBuffer->MapMemory(dataOffset, dataSize, 0, (void**)&mapped);
			memcpy(mapped, &constants.numSpheres, dataSize);
			mBuffer->UnmapMemory();

			dataOffset += dataSize;
			dataSize = spheres.size() * sizeof(SphereInfo);
			mBuffer->MapMemory(dataOffset, dataSize, 0, (void**)&mapped);
			memcpy(mapped, spheres.data(), dataSize);
			mBuffer->UnmapMemory();
		}

		virtual int GetSize()
		{
			return (NUM_MAX_SPHERES) * sizeof(Utopian::SphereInfo) + sizeof(constants);
		}

		struct {
			float numSpheres;
			glm::vec3 padding;
		} constants;

		// Note: Todo:
		std::array<Utopian::SphereInfo, NUM_MAX_SPHERES> spheres;
	};

	class GBufferJob : public BaseJob
	{
	public:
		UNIFORM_BLOCK_BEGIN(GBufferViewProjection)
			UNIFORM_PARAM(glm::mat4, projection)
			UNIFORM_PARAM(glm::mat4, view)
		UNIFORM_BLOCK_END()

		UNIFORM_BLOCK_BEGIN(SettingsBlock)
			UNIFORM_PARAM(int, normalMapping)
		UNIFORM_BLOCK_END()

		UNIFORM_BLOCK_BEGIN(AnimationParametersBlock)
			UNIFORM_PARAM(float, time)
			UNIFORM_PARAM(float, terrainSize)
			UNIFORM_PARAM(float, strength)
			UNIFORM_PARAM(float, frequency)
		UNIFORM_BLOCK_END()

		UNIFORM_BLOCK_BEGIN(FoliageSpheresBlock)
			UNIFORM_PARAM(float, numSpheres)
			UNIFORM_PARAM(glm::vec3, padding)
			UNIFORM_PARAM(SphereInfo, spheres[NUM_MAX_SPHERES])
		UNIFORM_BLOCK_END()

		struct InstancePushConstantBlock
		{
			InstancePushConstantBlock(float _modelHeight) {
				modelHeight = _modelHeight;
			}

			float modelHeight;
		};

		GBufferJob(Vk::Device* device, uint32_t width, uint32_t height);
		~GBufferJob();

		void Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer) override;
		void Render(const JobInput& jobInput) override;

		SharedPtr<Vk::RenderTarget> renderTarget;

		SharedPtr<Vk::GBufferEffect> mGBufferEffect;
		SharedPtr<Vk::GBufferEffect> mGBufferEffectWireframe;
		SharedPtr<Vk::Effect> mInstancedAnimationEffect;
		SharedPtr<Vk::Effect> mGBufferEffectInstanced;
	private:
		GBufferViewProjection viewProjectionBlock;
		SettingsBlock settingsBlock;
		SphereUniformBuffer foliageSpheresBlock;

		// Animated instancing
		AnimationParametersBlock animationParametersBlock;
		Vk::Texture* mWindmapTexture;
	};
}
