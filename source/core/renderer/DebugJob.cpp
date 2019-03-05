#include "core/renderer/DebugJob.h"
#include "core/renderer/DeferredJob.h"
#include "core/renderer/GBufferJob.h"
#include "core/renderer/SunShaftJob.h"
#include "core/renderer/CommonJobIncludes.h"

namespace Utopian
{
	DebugJob::DebugJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
	}

	DebugJob::~DebugJob()
	{
	}

	void DebugJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);
		SunShaftJob* sunShaftJob = static_cast<SunShaftJob*>(jobs[JobGraph::SUN_SHAFT_INDEX]);
		SetWaitSemaphore(sunShaftJob->GetCompletedSemahore());

		renderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
		renderTarget->AddColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ATTACHMENT_LOAD_OP_LOAD);
		//renderTarget->AddDepthAttachment(gbuffer.depthImage, VK_ATTACHMENT_LOAD_OP_LOAD);
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		normalEffect = Vk::gEffectManager().AddEffect<Vk::NormalDebugEffect>(mDevice, renderTarget->GetRenderPass());
		normalEffect->CreatePipeline();

		colorEffect = Vk::gEffectManager().AddEffect<Vk::ColorEffect>(mDevice, renderTarget->GetRenderPass());
		colorEffect->CreatePipeline();

		colorEffectWireframe = Vk::gEffectManager().AddEffect<Vk::ColorEffect>(mDevice, renderTarget->GetRenderPass());
		colorEffectWireframe->GetPipeline()->rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		colorEffectWireframe->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_NONE;
		colorEffectWireframe->GetPipeline()->inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		colorEffectWireframe->CreatePipeline();

		viewProjectionBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		colorEffect->BindUniformBuffer("UBO_viewProjection", &viewProjectionBlock);
		colorEffectWireframe->BindUniformBuffer("UBO_viewProjection", &viewProjectionBlock);
		normalEffect->BindUniformBuffer("UBO_viewProjection", &viewProjectionBlock);

		mCubeModel = Vk::gModelLoader().LoadDebugBoxLines();
	}

	void DebugJob::Render(const JobInput& jobInput)
	{
		viewProjectionBlock.data.view = jobInput.sceneInfo.viewMatrix;
		viewProjectionBlock.data.projection = jobInput.sceneInfo.projectionMatrix;
		viewProjectionBlock.UpdateMemory();

		renderTarget->Begin("Debug pass", glm::vec4(0.3, 0.6, 0.9, 1.0));
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
					commandBuffer->CmdBindDescriptorSets(colorEffect);
					commandBuffer->CmdPushConstants(colorEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConsts), &pushConsts);
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
					commandBuffer->CmdBindDescriptorSets(normalEffect);
					commandBuffer->CmdPushConstants(normalEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConsts), &pushConsts);
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
				commandBuffer->CmdBindDescriptorSets(colorEffectWireframe);
				commandBuffer->CmdPushConstants(colorEffectWireframe->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConsts), &pushConsts);
				commandBuffer->CmdBindVertexBuffer(0, 1, mCubeModel->mMeshes[0]->GetVertxBuffer());
				commandBuffer->CmdBindIndexBuffer(mCubeModel->mMeshes[0]->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
				commandBuffer->CmdDrawIndexed(mCubeModel->GetNumIndices(), 1, 0, 0, 0);
			}
		}

		renderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
	}
}
