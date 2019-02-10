#include "core/renderer/ShadowJob.h"
#include "core/renderer/BlurJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "vulkan/handles/FrameBuffers.h"
#include "vulkan/Debug.h"

namespace Utopian
{
	ShadowJob::ShadowJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		depthColorImage = std::make_shared<Vk::ImageColor>(device, SHADOWMAP_DIMENSION, SHADOWMAP_DIMENSION, VK_FORMAT_R32_SFLOAT, 4);
		depthImage = std::make_shared<Vk::ImageDepth>(device, SHADOWMAP_DIMENSION, SHADOWMAP_DIMENSION, VK_FORMAT_D32_SFLOAT_S8_UINT);

		renderTarget = std::make_shared<Vk::RenderTarget>(device, SHADOWMAP_DIMENSION, SHADOWMAP_DIMENSION);
		renderTarget->AddColorAttachment(depthColorImage->GetLayerView(0), depthColorImage->GetFormat());
		renderTarget->AddDepthAttachment(depthImage);
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		// Create multiple framebuffers with attachments to the individual array layers in the depth buffer image
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			SharedPtr<Vk::FrameBuffers> frameBuffer = std::make_shared<Vk::FrameBuffers>(device);
			frameBuffer->AddAttachmentImage(depthColorImage->GetLayerView(i));
			frameBuffer->AddAttachmentImage(depthImage.get());
			frameBuffer->Create(renderTarget->GetRenderPass(), SHADOWMAP_DIMENSION, SHADOWMAP_DIMENSION);
			mFrameBuffers.push_back(frameBuffer);
		}

		effect = Vk::gEffectManager().AddEffect<Vk::Effect>(device,
			renderTarget->GetRenderPass(),
			"data/shaders/shadowmap/shadowmap.vert",
			"data/shaders/shadowmap/shadowmap.frag");
		effect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		effect->CreatePipeline();

		effectInstanced = Vk::gEffectManager().AddEffect<Vk::Effect>(device,
			renderTarget->GetRenderPass(),
			"data/shaders/shadowmap/shadowmap_instancing.vert",
			"data/shaders/shadowmap/shadowmap.frag");
		effectInstanced->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_NONE;

		SharedPtr<Vk::VertexDescription> vertexDescription = std::make_shared<Vk::VertexDescription>(Vk::Vertex::GetDescription());
		vertexDescription->AddBinding(BINDING_1, sizeof(InstanceData), VK_VERTEX_INPUT_RATE_INSTANCE);
		vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());	// Location 0 : InInstanceWorld
		vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());	// Location 1 : InInstanceWorld
		vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());	// Location 2 : InInstanceWorld
		vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());	// Location 3 : InInstanceWorld
		effectInstanced->GetPipeline()->OverrideVertexInput(vertexDescription);

		effectInstanced->CreatePipeline();

		cascadeTransforms.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		effect->BindUniformBuffer("UBO_cascadeTransforms", &cascadeTransforms);
		effectInstanced->BindUniformBuffer("UBO_cascadeTransforms", &cascadeTransforms);

		const uint32_t size = 240;
		/*gScreenQuadUi().AddQuad(4 * (size + 10) + 10, height - (size + 10), size, size, depthColorImage->GetLayerView(0), renderTarget->GetSampler());
		gScreenQuadUi().AddQuad(5 * (size + 10) + 10, height - (size + 10), size, size, depthColorImage->GetLayerView(1), renderTarget->GetSampler());
		gScreenQuadUi().AddQuad(6 * (size + 10) + 10, height - (size + 10), size, size, depthColorImage->GetLayerView(2), renderTarget->GetSampler());
		gScreenQuadUi().AddQuad(4 * (size + 10) + 10, height - 2 * (size + 10), size, size, depthColorImage->GetLayerView(3), renderTarget->GetSampler());*/
	}

	ShadowJob::~ShadowJob()
	{
	}

	void ShadowJob::Init(const std::vector<BaseJob*>& jobs)
	{
		// In reality this job does not have to wait for the blur job
		BlurJob* blurJob = static_cast<BlurJob*>(jobs[JobGraph::BLUR_INDEX]);
		SetWaitSemaphore(blurJob->GetCompletedSemahore());
	}

	void ShadowJob::Render(const JobInput& jobInput)
	{
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			cascadeTransforms.data.viewProjection[i] = jobInput.sceneInfo.cascades[i].viewProjMatrix;
		}

		cascadeTransforms.UpdateMemory();

		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();
		commandBuffer->Begin();

		for (uint32_t cascadeIndex = 0; cascadeIndex < SHADOW_MAP_CASCADE_COUNT; cascadeIndex++)
		{
			// Begin the renderpass with the framebuffer attachments connected to the current cascade layer
			renderTarget->Begin(mFrameBuffers[cascadeIndex]->GetFrameBuffer(0), "Cascade pass", glm::vec4(1.0, 1.0, 0.0, 1.0));

			if (IsEnabled())
			{
				/* Render instanced assets */
				commandBuffer->CmdBindPipeline(effectInstanced->GetPipeline());

				for (uint32_t i = 0; i < jobInput.sceneInfo.instanceGroups.size(); i++)
				{
					SharedPtr<InstanceGroup> instanceGroup = jobInput.sceneInfo.instanceGroups[i];
					Vk::Buffer* instanceBuffer = instanceGroup->GetBuffer();
					Vk::StaticModel* model = instanceGroup->GetModel();

					if (instanceBuffer != nullptr && model != nullptr)
					{
						for (Vk::Mesh* mesh : model->mMeshes)
						{
							CascadePushConst pushConst(glm::mat4(), cascadeIndex);
							commandBuffer->CmdPushConstants(effectInstanced->GetPipelineInterface(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(CascadePushConst), &pushConst);

							VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptorSet();
							VkDescriptorSet descriptorSets[2] = { effectInstanced->GetDescriptorSet(0).GetVkHandle(), textureDescriptorSet };
							commandBuffer->CmdBindDescriptorSet(effectInstanced->GetPipelineInterface(), 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);

							commandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
							commandBuffer->CmdBindVertexBuffer(1, 1, instanceBuffer);
							commandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
							commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), instanceGroup->GetNumInstances(), 0, 0, 0);
						}
					}
				}

				commandBuffer->CmdBindPipeline(effect->GetPipeline());

				/* Render all renderables */
				for (auto& renderable : jobInput.sceneInfo.renderables)
				{
					if (!renderable->IsVisible() || ((renderable->GetRenderFlags() & RENDER_FLAG_DEFERRED) != RENDER_FLAG_DEFERRED))
						continue;

					Vk::StaticModel* model = renderable->GetModel();

					for (Vk::Mesh* mesh : model->mMeshes)
					{
						CascadePushConst pushConst(renderable->GetTransform().GetWorldMatrix(), cascadeIndex);
						commandBuffer->CmdPushConstants(effect->GetPipelineInterface(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(CascadePushConst), &pushConst);

						VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptorSet();
						VkDescriptorSet descriptorSets[2] = { effect->GetDescriptorSet(0).GetVkHandle(), textureDescriptorSet };
						commandBuffer->CmdBindDescriptorSet(effect->GetPipelineInterface(), 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);

						commandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
						commandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
						commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
					}
				}
			}

			commandBuffer->CmdEndRenderPass();
			Vk::DebugMarker::EndRegion(commandBuffer->GetVkHandle());
		}

		commandBuffer->Submit(GetWaitSemahore(), GetCompletedSemahore());
	}
}
