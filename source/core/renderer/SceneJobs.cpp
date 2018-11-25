#include "core/renderer/SceneJobs.h"
#include "core/renderer/Renderable.h"
#include "core/renderer/RenderingManager.h"
#include "vulkan/Renderer.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/BasicRenderTarget.h"
#include "vulkan/StaticModel.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/Pipeline.h"
#include "vulkan/handles/RenderPass.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/ScreenGui.h"
#include "vulkan/EffectManager.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/CubeMapTexture.h"
#include <random>

namespace Utopian
{
	GBufferJob::GBufferJob(Vk::Renderer* renderer, uint32_t width, uint32_t height)
		: BaseJob(renderer, width, height)
	{
		positionImage = std::make_shared<Vk::ImageColor>(renderer->GetDevice(), width, height, VK_FORMAT_R32G32B32A32_SFLOAT);
		normalImage = std::make_shared<Vk::ImageColor>(renderer->GetDevice(), width, height, VK_FORMAT_R16G16B16A16_SFLOAT);
		normalViewImage = std::make_shared<Vk::ImageColor>(renderer->GetDevice(), width, height, VK_FORMAT_R8G8B8A8_UNORM);
		albedoImage = std::make_shared<Vk::ImageColor>(renderer->GetDevice(), width, height, VK_FORMAT_R8G8B8A8_UNORM);
		depthImage = std::make_shared<Vk::ImageDepth>(renderer->GetDevice(), width, height, VK_FORMAT_D32_SFLOAT_S8_UINT);

		renderTarget = std::make_shared<Vk::RenderTarget>(renderer->GetDevice(), renderer->GetCommandPool(), width, height);
		renderTarget->AddColorAttachment(positionImage);
		renderTarget->AddColorAttachment(normalImage);
		renderTarget->AddColorAttachment(albedoImage);
		renderTarget->AddColorAttachment(normalViewImage);
		renderTarget->AddDepthAttachment(depthImage);
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		// Todo: Implement a better way for multiple pipelines in the same Effect
		mGBufferEffect = Vk::gEffectManager().AddEffect<Vk::GBufferEffect>(renderer->GetDevice(), renderTarget->GetRenderPass());
		mGBufferEffectWireframe = Vk::gEffectManager().AddEffect<Vk::GBufferEffect>(renderer->GetDevice(), renderTarget->GetRenderPass());
		mGBufferEffectWireframe->GetPipeline()->rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;

		mGBufferEffect->CreatePipeline();
		mGBufferEffectWireframe->CreatePipeline();

		const uint32_t size = 240;
		renderer->AddScreenQuad(size + 20, height - (size + 10), size, size, positionImage.get(), renderTarget->GetSampler());
		renderer->AddScreenQuad(2 * (size + 10) + 10, height - (size + 10), size, size, normalImage.get(), renderTarget->GetSampler());
		renderer->AddScreenQuad(3 * (size + 10) + 10, height - (size + 10), size, size, albedoImage.get(), renderTarget->GetSampler());
	}

	GBufferJob::~GBufferJob()
	{
	}

	void GBufferJob::Init(const std::vector<BaseJob*>& jobs)
	{
	}

	void GBufferJob::Render(Vk::Renderer* renderer, const JobInput& jobInput)
	{
		mGBufferEffect->SetCameraData(jobInput.sceneInfo.viewMatrix, jobInput.sceneInfo.projectionMatrix);
		mGBufferEffectWireframe->SetCameraData(jobInput.sceneInfo.viewMatrix, jobInput.sceneInfo.projectionMatrix);

		renderTarget->Begin();
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		/* Render all renderables */
		for (auto& renderable : jobInput.sceneInfo.renderables)
		{
			if (!renderable->IsVisible() || ((renderable->GetRenderFlags() & RENDER_FLAG_DEFERRED) != RENDER_FLAG_DEFERRED))
				continue;

			Vk::Effect* effect = mGBufferEffect.get();
			if (renderable->HasRenderFlags(RENDER_FLAG_WIREFRAME))
				effect = mGBufferEffectWireframe.get();

			Vk::StaticModel* model = renderable->GetModel();

			for (Vk::Mesh* mesh : model->mMeshes)
			{
				// Push the world matrix constant
				Vk::PushConstantBlock pushConsts(renderable->GetTransform().GetWorldMatrix(), renderable->GetColor(), renderable->GetTextureTiling());

				// Todo: Note: This is a temporary workaround
				VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptor();
				VkDescriptorSet descriptorSets[2] = { effect->GetDescriptorSet(0).descriptorSet, textureDescriptorSet };

				commandBuffer->CmdBindPipeline(effect->GetPipeline());
				commandBuffer->CmdBindDescriptorSet(effect->GetPipelineInterface(), 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);
				commandBuffer->CmdPushConstants(effect->GetPipelineInterface(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConsts), &pushConsts);
				commandBuffer->CmdBindVertexBuffer(0, 1, &mesh->vertices.buffer);
				commandBuffer->CmdBindIndexBuffer(mesh->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
				commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
			}
		}

		renderTarget->End(renderer->GetQueue());
	}

	DeferredJob::DeferredJob(Vk::Renderer* renderer, uint32_t width, uint32_t height)
		: BaseJob(renderer, width, height)
	{
		renderTarget = std::make_shared<Vk::BasicRenderTarget>(renderer->GetDevice(), renderer->GetCommandPool(), width, height, VK_FORMAT_R8G8B8A8_UNORM);
		effect = Vk::gEffectManager().AddEffect<Vk::DeferredEffect>(renderer->GetDevice(), renderTarget->GetRenderPass());

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
		: BaseJob(renderer, width, height)
	{
		ssaoImage = std::make_shared<Vk::ImageColor>(renderer->GetDevice(), width, height, VK_FORMAT_R16G16B16A16_SFLOAT);

		renderTarget = std::make_shared<Vk::RenderTarget>(renderer->GetDevice(), renderer->GetCommandPool(), width, height);
		renderTarget->AddColorAttachment(ssaoImage);
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		effect = Vk::gEffectManager().AddEffect<Vk::SSAOEffect>(renderer->GetDevice(), renderTarget->GetRenderPass());

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

			effect->cameraBlock.data.samples[i] = glm::vec4(sample, 0);
		}

		effect->UpdateMemory();
	}

	BlurJob::BlurJob(Vk::Renderer* renderer, uint32_t width, uint32_t height)
		: BaseJob(renderer, width, height)
	{
		blurImage = std::make_shared<Vk::ImageColor>(renderer->GetDevice(), width, height, VK_FORMAT_R16G16B16A16_SFLOAT);

		renderTarget = std::make_shared<Vk::RenderTarget>(renderer->GetDevice(), renderer->GetCommandPool(), width, height);
		renderTarget->AddColorAttachment(blurImage);
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		effect = Vk::gEffectManager().AddEffect<Vk::BlurEffect>(renderer->GetDevice(), renderTarget->GetRenderPass());

		const uint32_t size = 240;
		renderer->AddScreenQuad(10, height - (size + 10), size, size, blurImage.get(), renderTarget->GetSampler());
	}

	BlurJob::~BlurJob()
	{
	}

	void BlurJob::Init(const std::vector<BaseJob*>& renderers)
	{
		SSAOJob* ssaoJob = static_cast<SSAOJob*>(renderers[1]);
		effect->BindSSAOOutput(ssaoJob->ssaoImage.get(), ssaoJob->renderTarget->GetSampler());
	}

	void BlurJob::Render(Vk::Renderer* renderer, const JobInput& jobInput)
	{
		effect->SetSettings(jobInput.renderingSettings.blurRadius);

		renderTarget->Begin();
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		// Todo: Should this be moved to the effect instead?
		commandBuffer->CmdBindPipeline(effect->GetPipeline());
		effect->BindDescriptorSets(commandBuffer);

		renderer->DrawScreenQuad(commandBuffer);

		renderTarget->End(renderer->GetQueue());
	}

	SkyboxJob::SkyboxJob(Vk::Renderer* renderer, uint32_t width, uint32_t height)
		: BaseJob(renderer, width, height)
	{
	}

	SkyboxJob::~SkyboxJob()
	{
	}

	void SkyboxJob::Init(const std::vector<BaseJob*>& renderers)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(renderers[3]);
		GBufferJob* gbufferJob = static_cast<GBufferJob*>(renderers[0]);

		renderTarget = std::make_shared<Vk::RenderTarget>(mRenderer->GetDevice(), mRenderer->GetCommandPool(), mWidth, mHeight);
		renderTarget->AddColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ATTACHMENT_LOAD_OP_LOAD);
		renderTarget->AddDepthAttachment(gbufferJob->depthImage);
		// Todo: Investigate why this does not work
		renderTarget->GetRenderPass()->attachments[Vk::RenderPassAttachment::DEPTH_ATTACHMENT].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		skybox = std::make_shared<Vk::CubeMapTexture>();
		skybox->LoadFromFile("data/textures/cubemap_space.ktx", VK_FORMAT_R8G8B8A8_UNORM, mRenderer->GetDevice(), mRenderer->GetQueue());

		effect = Vk::gEffectManager().AddEffect<Vk::SkyboxEffect>(mRenderer->GetDevice(), renderTarget->GetRenderPass());
		effect->BindCombinedImage("samplerCubeMap", skybox->image, renderTarget->GetSampler()); // skybox->sampler);

		mCubeModel = Vk::gModelLoader().LoadDebugBox();
	}

	void SkyboxJob::Render(Vk::Renderer* renderer, const JobInput& jobInput)
	{
		effect->SetCameraData(jobInput.sceneInfo.viewMatrix, jobInput.sceneInfo.projectionMatrix);

		renderTarget->Begin();
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		// Todo: Should this be moved to the effect instead?
		commandBuffer->CmdBindPipeline(effect->GetPipeline());
		effect->BindDescriptorSets(commandBuffer);

		commandBuffer->CmdBindVertexBuffer(0, 1, &mCubeModel->mMeshes[0]->vertices.buffer);
		commandBuffer->CmdBindIndexBuffer(mCubeModel->mMeshes[0]->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
		commandBuffer->CmdDrawIndexed(mCubeModel->GetNumIndices(), 1, 0, 0, 0);

		renderTarget->End(renderer->GetQueue());
	}

	DebugJob::DebugJob(Vk::Renderer* renderer, uint32_t width, uint32_t height)
		: BaseJob(renderer, width, height)
	{
	}

	DebugJob::~DebugJob()
	{
	}

	void DebugJob::Init(const std::vector<BaseJob*>& renderers)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(renderers[3]);
		GBufferJob* gbufferJob = static_cast<GBufferJob*>(renderers[0]);

		renderTarget = std::make_shared<Vk::RenderTarget>(mRenderer->GetDevice(), mRenderer->GetCommandPool(), mWidth, mHeight);
		renderTarget->AddColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ATTACHMENT_LOAD_OP_LOAD);
		renderTarget->AddDepthAttachment(gbufferJob->depthImage);
		// Todo: Investigate why this does not work
		//renderTarget->GetRenderPass()->attachments[Vk::RenderPassAttachment::DEPTH_ATTACHMENT].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		colorEffect = Vk::gEffectManager().AddEffect<Vk::ColorEffect>(mRenderer->GetDevice(), renderTarget->GetRenderPass());
		colorEffect->CreatePipeline();
		normalEffect = Vk::gEffectManager().AddEffect<Vk::NormalDebugEffect>(mRenderer->GetDevice(), renderTarget->GetRenderPass());

		colorEffectWireframe = Vk::gEffectManager().AddEffect<Vk::ColorEffect>(mRenderer->GetDevice(), renderTarget->GetRenderPass());
		colorEffectWireframe->GetPipeline()->rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		colorEffectWireframe->CreatePipeline();

		//mScreenQuad = mRenderer->AddScreenQuad(0u, 0u, mWidth, mHeight, renderTarget->GetColorImage(), renderTarget->GetSampler(), 1u);

		mCubeModel = Vk::gModelLoader().LoadDebugBox();
	}

	void DebugJob::Render(Vk::Renderer* renderer, const JobInput& jobInput)
	{
		colorEffect->SetCameraData(jobInput.sceneInfo.viewMatrix, jobInput.sceneInfo.projectionMatrix);
		normalEffect->SetCameraData(jobInput.sceneInfo.viewMatrix, jobInput.sceneInfo.projectionMatrix);
		colorEffectWireframe->SetCameraData(jobInput.sceneInfo.viewMatrix, jobInput.sceneInfo.projectionMatrix);

		renderTarget->Begin();
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		/* Render all renderables */
		for (auto& renderable : jobInput.sceneInfo.renderables)
		{
			if (!renderable->IsVisible())
				continue;

			Vk::StaticModel* model = renderable->GetModel();

			if (renderable->HasRenderFlags(RENDER_FLAG_COLOR))
			{
				for (Vk::Mesh* mesh : model->mMeshes)
				{
					// Push the world matrix constant
					Vk::PushConstantBlock pushConsts(renderable->GetTransform().GetWorldMatrix(), renderable->GetColor());

					commandBuffer->CmdBindPipeline(colorEffect->GetPipeline());
					colorEffect->BindDescriptorSets(commandBuffer);
					commandBuffer->CmdPushConstants(colorEffect->GetPipelineInterface(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConsts), &pushConsts);
					commandBuffer->CmdBindVertexBuffer(0, 1, &mesh->vertices.buffer);
					commandBuffer->CmdBindIndexBuffer(mesh->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
					commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
				}
			}
			
			if (renderable->HasRenderFlags(RENDER_FLAG_NORMAL_DEBUG))
			{
				for (Vk::Mesh* mesh : model->mMeshes)
				{
					// Push the world matrix constant
					Vk::PushConstantBlock pushConsts(renderable->GetTransform().GetWorldMatrix(), renderable->GetColor());

					commandBuffer->CmdBindPipeline(normalEffect->GetPipeline());
					normalEffect->BindDescriptorSets(commandBuffer);
					commandBuffer->CmdPushConstants(normalEffect->GetPipelineInterface(), VK_SHADER_STAGE_GEOMETRY_BIT, sizeof(pushConsts), &pushConsts);
					commandBuffer->CmdBindVertexBuffer(0, 1, &mesh->vertices.buffer);
					commandBuffer->CmdBindIndexBuffer(mesh->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
					commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
				}
			}

			if (renderable->HasRenderFlags(RENDER_FLAG_BOUNDING_BOX))
			{
				BoundingBox boundingBox = renderable->GetBoundingBox();
				glm::vec3 pos = renderable->GetTransform().GetPosition();
				glm::vec3 rotation = renderable->GetTransform().GetRotation();
				glm::vec3 translation = glm::vec3(pos.x, boundingBox.GetMin().y + boundingBox.GetHeight()/2, pos.z);
				glm::mat4 world = glm::translate(glm::mat4(), translation);
				world = glm::scale(world, glm::vec3(boundingBox.GetWidth(), boundingBox.GetHeight(), boundingBox.GetDepth()));

				Vk::PushConstantBlock pushConsts(world, glm::vec4(0, 1, 0, 1));

				commandBuffer->CmdBindPipeline(colorEffectWireframe->GetPipeline());
				colorEffectWireframe->BindDescriptorSets(commandBuffer);
				commandBuffer->CmdPushConstants(colorEffectWireframe->GetPipelineInterface(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConsts), &pushConsts);
				commandBuffer->CmdBindVertexBuffer(0, 1, &mCubeModel->mMeshes[0]->vertices.buffer);
				commandBuffer->CmdBindIndexBuffer(mCubeModel->mMeshes[0]->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
				commandBuffer->CmdDrawIndexed(mCubeModel->GetNumIndices(), 1, 0, 0, 0);
			}
		}

		renderTarget->End(renderer->GetQueue());
	}
}
