#include "core/renderer/SceneJobs.h"
#include "core/renderer/Renderable.h"
#include "vulkan/Renderer.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/BasicRenderTarget.h"
#include "vulkan/StaticModel.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/ScreenGui.h"
#include <random>

namespace Utopian
{
	GBufferJob::GBufferJob(Vk::Renderer* renderer, uint32_t width, uint32_t height)
	{
		positionImage = make_shared<Vk::ImageColor>(renderer->GetDevice(), width, height, VK_FORMAT_R32G32B32A32_SFLOAT);
		normalImage = make_shared<Vk::ImageColor>(renderer->GetDevice(), width, height, VK_FORMAT_R8G8B8A8_UNORM);
		normalViewImage = make_shared<Vk::ImageColor>(renderer->GetDevice(), width, height, VK_FORMAT_R8G8B8A8_UNORM);
		albedoImage = make_shared<Vk::ImageColor>(renderer->GetDevice(), width, height, VK_FORMAT_R8G8B8A8_UNORM);
		depthImage = make_shared<Vk::ImageDepth>(renderer->GetDevice(), width, height, VK_FORMAT_D32_SFLOAT_S8_UINT);

		renderTarget = make_shared<Vk::RenderTarget>(renderer->GetDevice(), renderer->GetCommandPool(), width, height);
		renderTarget->AddColorAttachment(positionImage);
		renderTarget->AddColorAttachment(normalImage);
		renderTarget->AddColorAttachment(albedoImage);
		renderTarget->AddColorAttachment(normalViewImage);
		renderTarget->AddDepthAttachment(depthImage);
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		mGBufferEffect.SetRenderPass(renderTarget->GetRenderPass());
		mGBufferEffect.Init(renderer);

		renderer->AddScreenQuad(width - 350 - 50, height - 350, 300, 300, positionImage.get(), renderTarget->GetSampler());
		renderer->AddScreenQuad(width - 2*350 - 50, height - 350, 300, 300, normalImage.get(), renderTarget->GetSampler());
		renderer->AddScreenQuad(width - 3*350 - 50, height - 350, 300, 300, albedoImage.get(), renderTarget->GetSampler());
	}

	GBufferJob::~GBufferJob()
	{
	}

	void GBufferJob::Init(const std::vector<BaseJob*>& jobs)
	{
	}

	void GBufferJob::Render(Vk::Renderer* renderer, const JobInput& jobInput)
	{
		mGBufferEffect.SetCameraData(jobInput.sceneInfo.viewMatrix, jobInput.sceneInfo.projectionMatrix);

		renderTarget->Begin();
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		/* Render all renderables */
		for (auto& renderable : jobInput.sceneInfo.renderables)
		{
			Vk::StaticModel* model = renderable->GetModel();

			for (Vk::Mesh* mesh : model->mMeshes)
			{
				// Push the world matrix constant
				Vk::PushConstantBlock pushConsts(renderable->GetTransform().GetWorldMatrix(), renderable->GetColor());

				VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptor();
				VkDescriptorSet descriptorSets[2] = { mGBufferEffect.mDescriptorSet0->descriptorSet, textureDescriptorSet };

				commandBuffer->CmdBindPipeline(mGBufferEffect.GetPipeline(0));
				commandBuffer->CmdBindDescriptorSet(&mGBufferEffect, 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);
				commandBuffer->CmdPushConstants(&mGBufferEffect, VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConsts), &pushConsts);
				commandBuffer->CmdBindVertexBuffer(0, 1, &mesh->vertices.buffer);
				commandBuffer->CmdBindIndexBuffer(mesh->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
				commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
			}
		}

		renderTarget->End(renderer->GetQueue());
	}

	DeferredJob::DeferredJob(Vk::Renderer* renderer, uint32_t width, uint32_t height)
	{
		renderTarget = make_shared<Vk::BasicRenderTarget>(renderer->GetDevice(), renderer->GetCommandPool(), width, height, VK_FORMAT_R8G8B8A8_UNORM);
		effect = make_unique<Vk::DeferredEffect>(renderer->GetDevice(), renderTarget->GetRenderPass());

		mScreenQuad = renderer->AddScreenQuad(0u, 0u, width, height, renderTarget->GetColorImage(), renderTarget->GetSampler(), 1u);
	}

	DeferredJob::~DeferredJob()
	{
	}

	void DeferredJob::Init(const std::vector<BaseJob*>& jobs)
	{
		GBufferJob* gbufferJob = static_cast<GBufferJob*>(jobs[0]);
		BlurJob* blurJob = static_cast<BlurJob*>(jobs[2]);
		effect->BindImages(gbufferJob->positionImage.get(),
						   gbufferJob->normalImage.get(),
						   gbufferJob->albedoImage.get(),
						   blurJob->blurImage.get(),
						   gbufferJob->renderTarget->GetSampler());
	}

	void DeferredJob::Render(Vk::Renderer* renderer, const JobInput& jobInput)
	{
		mScreenQuad->SetVisible(jobInput.renderingSettings.deferredPipeline);

		effect->SetFogData(jobInput.renderingSettings);
		effect->SetEyePos(glm::vec4(jobInput.sceneInfo.eyePos, 1.0f));
		effect->SetLightArray(jobInput.sceneInfo.lights);

		renderTarget->Begin();
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		// Todo: Should this be moved to the effect instead?
		commandBuffer->CmdBindPipeline(effect->GetPipeline());
		effect->BindDescriptorSets(commandBuffer);

		renderer->DrawScreenQuad(commandBuffer);

		renderTarget->End(renderer->GetQueue());
	}

	SSAOJob::SSAOJob(Vk::Renderer* renderer, uint32_t width, uint32_t height)
	{
		ssaoImage = make_shared<Vk::ImageColor>(renderer->GetDevice(), width, height, VK_FORMAT_R16G16B16A16_SFLOAT);

		renderTarget = make_shared<Vk::RenderTarget>(renderer->GetDevice(), renderer->GetCommandPool(), width, height);
		renderTarget->AddColorAttachment(ssaoImage);
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		effect = make_unique<Vk::SSAOEffect>(renderer->GetDevice(), renderTarget->GetRenderPass());

		//renderer->AddScreenQuad(width - 650 - 50, height - 950, 600, 600, ssaoImage.get(), renderTarget->GetSampler());
	}

	SSAOJob::~SSAOJob()
	{
	}

	void SSAOJob::Init(const std::vector<BaseJob*>& renderers)
	{
		GBufferJob* gbufferRenderer = static_cast<GBufferJob*>(renderers[0]);
		effect->BindGBuffer(gbufferRenderer->positionImage.get(),
						   gbufferRenderer->normalViewImage.get(),
						   gbufferRenderer->albedoImage.get(),
						   gbufferRenderer->renderTarget->GetSampler());

		CreateKernelSamples();
	}

	void SSAOJob::Render(Vk::Renderer* renderer, const JobInput& jobInput)
	{
		effect->SetCameraData(jobInput.sceneInfo.viewMatrix, jobInput.sceneInfo.projectionMatrix, glm::vec4(jobInput.sceneInfo.eyePos, 1.0f));
		effect->SetSettings(jobInput.renderingSettings.ssaoRadius, jobInput.renderingSettings.ssaoBias);

		renderTarget->Begin();
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		// Todo: Should this be moved to the effect instead?
		commandBuffer->CmdBindPipeline(effect->GetPipeline());
		effect->BindDescriptorSets(commandBuffer);

		renderer->DrawScreenQuad(commandBuffer);

		renderTarget->End(renderer->GetQueue());
	}

	void SSAOJob::CreateKernelSamples()
	{
		// Kernel samples
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
		std::default_random_engine generator;
		for (unsigned int i = 0; i < 64; ++i)
		{
			glm::vec3 sample(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) 
			);
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);

			effect->cameraBlock.data.samples[i] = vec4(sample, 0);
		}

		effect->UpdateMemory();
	}

	BlurJob::BlurJob(Vk::Renderer* renderer, uint32_t width, uint32_t height)
	{
		blurImage = make_shared<Vk::ImageColor>(renderer->GetDevice(), width, height, VK_FORMAT_R16G16B16A16_SFLOAT);

		renderTarget = make_shared<Vk::RenderTarget>(renderer->GetDevice(), renderer->GetCommandPool(), width, height);
		renderTarget->AddColorAttachment(blurImage);
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		effect.SetRenderPass(renderTarget->GetRenderPass());
		effect.Init(renderer);

		renderer->AddScreenQuad(width - 650 - 50, height - 950, 600, 600, blurImage.get(), renderTarget->GetSampler());
	}

	BlurJob::~BlurJob()
	{
	}

	void BlurJob::Init(const std::vector<BaseJob*>& renderers)
	{
		SSAOJob* ssaoJob = static_cast<SSAOJob*>(renderers[1]);
		effect.BindSSAOOutput(ssaoJob->ssaoImage.get(), ssaoJob->renderTarget->GetSampler());
	}

	void BlurJob::Render(Vk::Renderer* renderer, const JobInput& jobInput)
	{
		effect.SetSettings(jobInput.renderingSettings.blurRadius);

		renderTarget->Begin();
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		// Todo: Should this be moved to the effect instead?
		commandBuffer->CmdBindPipeline(effect.GetPipeline(0));
		effect.BindDescriptorSets(commandBuffer);

		renderer->DrawScreenQuad(commandBuffer);

		renderTarget->End(renderer->GetQueue());
	}
}
