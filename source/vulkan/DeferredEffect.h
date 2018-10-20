#pragma once

#include <glm/glm.hpp>
#include "vulkan/Effect.h"
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/PipelineInterface.h"
#include "core/CommonBuffers.h"

namespace Utopian
{
	class Light;
	struct RenderingSettings;
}

namespace Utopian::Vk
{
	class Renderer;
	class DescriptorSetLayout;
	class DescriptorPool;
	class DescriptorSet;
	class Pipeline2;
	class ComputePipeline;
	class PipelineLayout;
	class VertexDescription;
	class Shader;
	class Texture;

	class DeferredEffect : public Effect
	{
	public:
		enum Variation {
			NORMAL = 0
		};

		class UniformBufferPS : public Utopian::Vk::ShaderBuffer
		{
		public:
			virtual void UpdateMemory() {
				// Map uniform buffer and update it
				uint8_t *mapped;
				mBuffer->MapMemory(0, sizeof(data), 0, (void**)&mapped);
				memcpy(mapped, &data, sizeof(data));
				mBuffer->UnmapMemory();
			}

			virtual int GetSize() {
				return sizeof(data);
			}

			struct {
				glm::vec4 eyePos;
			} data;
		};

		DeferredEffect();

		void SetEyePos(glm::vec3 eyePos);
		void BindImages(Image* positionImage, Image* normalImage, Image* albedoImage, Image* ssaoImage, Sampler* sampler);
		void SetLightArray(const std::vector<Light*>& lights);
		void SetFogData(const RenderingSettings& renderingSettings);
		void BindDescriptorSets(CommandBuffer* commandBuffer);

		// Override the base class interfaces
		virtual void CreateDescriptorPool(Device* device);
		virtual void CreateVertexDescription(Device* device);
		virtual void CreatePipelineInterface(Device* device);
		virtual void CreateDescriptorSets(Device* device);
		virtual void CreatePipeline(Renderer* renderer);
		virtual void UpdateMemory();

		DescriptorSet* mDescriptorSet0;
		DescriptorSet* mDescriptorSet1;
		UniformBufferPS eye_ubo;
		LightUniformBuffer light_ubo;
		FogUniformBuffer fog_ubo;
	private:
	};
}
