#pragma once
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
	class Pipeline3;
	class ComputePipeline;
	class PipelineLayout;
	class VertexDescription;
	class Shader;
	class Texture;

	class SSAOEffect : public Effect
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
				glm::mat4 projection;
				glm::mat4 view;
				glm::vec4 eyePos;
				glm::vec4 samples[64];
			} data;
		};

		class SettingsUniformBufferPS : public Utopian::Vk::ShaderBuffer
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
				float radius;
				float bias;
			} data;
		};

		SSAOEffect();

		void SetEyePos(glm::vec3 eyePos);
		void SetCameraData(glm::mat4 view, glm::mat4 projection);
		void SetSettings(float radius, float bias);

		// Note: the normal image contains normals in view space
		void BindGBuffer(Image* positionImage, Image* normalViewImage, Image* albedoImage, Sampler* sampler);
		void BindDescriptorSets(CommandBuffer* commandBuffer);

		void SetRenderPass(RenderPass* renderPass);

		// Override the base class interfaces
		virtual void CreateDescriptorPool(Device* device);
		virtual void CreateVertexDescription(Device* device);
		virtual void CreatePipelineInterface(Device* device);
		virtual void CreateDescriptorSets(Device* device);
		virtual void CreatePipeline(Renderer* renderer);
		virtual void UpdateMemory();

		VkPipeline GetVkPipeline();

		DescriptorSet* mDescriptorSet0;
		DescriptorSet* mDescriptorSet1;
		DescriptorSet* mDescriptorSet2;
		Utopian::Vk::Texture* noiseTexture;
		RenderPass* mRenderPass;

		UniformBufferPS ubo;
		SettingsUniformBufferPS ubo_settings;
	private:
		Pipeline3* mPipeline;
	};
}
