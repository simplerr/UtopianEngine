#include "core/renderer/SceneRenderers.h"
#include "core/renderer/Renderable.h"
#include "vulkan/Renderer.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/BasicRenderTarget.h"
#include "vulkan/StaticModel.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/CommandBuffer.h"

namespace Utopian
{
	GBufferRenderer::GBufferRenderer(Vk::Renderer* renderer, uint32_t width, uint32_t height)
	{
		positionImage = new Vk::ImageColor(renderer->GetDevice(), width, height, VK_FORMAT_R16G16B16A16_SFLOAT);
		normalImage = new Vk::ImageColor(renderer->GetDevice(), width, height, VK_FORMAT_R16G16B16A16_SFLOAT);
		albedoImage = new Vk::ImageColor(renderer->GetDevice(), width, height, VK_FORMAT_R8G8B8A8_UNORM);
		depthImage = new Vk::ImageDepth(renderer->GetDevice(), width, height, VK_FORMAT_D32_SFLOAT_S8_UINT);

		renderTarget = new Vk::RenderTarget(renderer->GetDevice(), renderer->GetCommandPool(), width, height);
		renderTarget->AddColorAttachment(positionImage);
		renderTarget->AddColorAttachment(normalImage);
		renderTarget->AddColorAttachment(albedoImage);
		renderTarget->AddDepthAttachment(depthImage);
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		mGBufferEffect.SetRenderPass(renderTarget->GetRenderPass());
		mGBufferEffect.Init(renderer);
	}

	GBufferRenderer::~GBufferRenderer()
	{
		delete positionImage;
		delete normalImage;
		delete albedoImage;
		delete depthImage;
		delete renderTarget;
	}

	void GBufferRenderer::render(Vk::Renderer* renderer, const SceneInfo& sceneInfo)
	{
		mGBufferEffect.per_frame_vs.data.projection = sceneInfo.projectionMatrix;
		mGBufferEffect.per_frame_vs.data.view = sceneInfo.viewMatrix;
		mGBufferEffect.UpdateMemory(renderer->GetDevice());

		renderTarget->Begin();
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		for (auto& renderable : sceneInfo.renderables)
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

	DeferredRenderer::DeferredRenderer(Vk::Renderer* renderer)
	{
	}

	DeferredRenderer::~DeferredRenderer()
	{
	}

	void DeferredRenderer::render(Vk::Renderer* renderer, const SceneInfo& sceneInfo)
	{
	}
}
