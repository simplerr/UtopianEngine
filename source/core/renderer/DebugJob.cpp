#include "core/renderer/DebugJob.h"
#include "core/renderer/DeferredJob.h"
#include "core/renderer/GBufferJob.h"
#include "core/renderer/CommonJobIncludes.h"

namespace Utopian
{
	DebugJob::DebugJob(Vk::Renderer* renderer, uint32_t width, uint32_t height)
		: BaseJob(renderer, width, height)
	{
	}

	DebugJob::~DebugJob()
	{
	}

	void DebugJob::Init(const std::vector<BaseJob*>& renderers)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(renderers[RenderingManager::DEFERRED_INDEX]);
		GBufferJob* gbufferJob = static_cast<GBufferJob*>(renderers[RenderingManager::GBUFFER_INDEX]);

		renderTarget = std::make_shared<Vk::RenderTarget>(mRenderer->GetDevice(), mWidth, mHeight);
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
					commandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
					commandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
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
					commandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
					commandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
					commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
				}
			}

			if (renderable->HasRenderFlags(RENDER_FLAG_BOUNDING_BOX))
			{
				BoundingBox boundingBox = renderable->GetBoundingBox();
				glm::vec3 pos = renderable->GetTransform().GetPosition();
				glm::vec3 rotation = renderable->GetTransform().GetRotation();
				glm::vec3 translation = glm::vec3(pos.x, boundingBox.GetMin().y + boundingBox.GetHeight() / 2, pos.z);
				glm::mat4 world = glm::translate(glm::mat4(), translation);
				world = glm::scale(world, glm::vec3(boundingBox.GetWidth(), boundingBox.GetHeight(), boundingBox.GetDepth()));

				Vk::PushConstantBlock pushConsts(world, glm::vec4(0, 1, 0, 1));

				commandBuffer->CmdBindPipeline(colorEffectWireframe->GetPipeline());
				colorEffectWireframe->BindDescriptorSets(commandBuffer);
				commandBuffer->CmdPushConstants(colorEffectWireframe->GetPipelineInterface(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConsts), &pushConsts);
				commandBuffer->CmdBindVertexBuffer(0, 1, mCubeModel->mMeshes[0]->GetVertxBuffer());
				commandBuffer->CmdBindIndexBuffer(mCubeModel->mMeshes[0]->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
				commandBuffer->CmdDrawIndexed(mCubeModel->GetNumIndices(), 1, 0, 0, 0);
			}
		}

		renderTarget->End();
	}
}
