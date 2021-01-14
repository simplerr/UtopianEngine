#include "core/renderer/jobs/ShadowJob.h"
#include "core/renderer/jobs/BlurJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "vulkan/handles/FrameBuffers.h"
#include "vulkan/handles/QueryPoolTimestamp.h"
#include "vulkan/Debug.h"
#include "core/Profiler.h"

namespace Utopian
{
	ShadowJob::ShadowJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		depthColorImage = std::make_shared<Vk::ImageColor>(device, SHADOWMAP_DIMENSION, SHADOWMAP_DIMENSION, VK_FORMAT_R32_SFLOAT, "Shadow depth color image", 4);
		mDepthImage = std::make_shared<Vk::ImageDepth>(device, SHADOWMAP_DIMENSION, SHADOWMAP_DIMENSION, VK_FORMAT_D32_SFLOAT_S8_UINT, "Shadow depth image");

		mCommandBuffer = std::make_shared<Vk::CommandBuffer>(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY, false);

		mRenderPass = std::make_shared<Vk::RenderPass>(device);
		mRenderPass->AddColorAttachment(depthColorImage->GetFormat(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		mRenderPass->AddDepthAttachment(mDepthImage->GetFormat());
		mRenderPass->Create();

      for (uint32_t i = 0; i < mRenderPass->GetNumColorAttachments(); i++)
      {
         VkClearValue clearValue;
         clearValue.color = {1.0f, 1.0f, 1.0f, 1.0f};
         mClearValues.push_back(clearValue);
      }

      // Note: Always assumes that the depth stencil attachment is the last one
      VkClearValue clearValue;
      clearValue.depthStencil = { 1.0f, 0 };
      mClearValues.push_back(clearValue);

		// Create multiple framebuffers with attachments to the individual array layers in the depth buffer image
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			SharedPtr<Vk::FrameBuffers> frameBuffer = std::make_shared<Vk::FrameBuffers>(device);
			frameBuffer->AddAttachmentImage(depthColorImage->GetLayerView(i));
			frameBuffer->AddAttachmentImage(mDepthImage.get());
			frameBuffer->Create(mRenderPass.get(), SHADOWMAP_DIMENSION, SHADOWMAP_DIMENSION);
			mFrameBuffers.push_back(frameBuffer);
		}

		Vk::EffectCreateInfo effectDesc;
		effectDesc.shaderDesc.vertexShaderPath = "data/shaders/shadowmap/shadowmap.vert";
		effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/shadowmap/shadowmap.frag";
		effectDesc.pipelineDesc.rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(device, mRenderPass.get(), effectDesc);

		// Custom vertex description due to instancing
		SharedPtr<Vk::VertexDescription> vertexDescription = std::make_shared<Vk::VertexDescription>(Vk::Vertex::GetDescription());
		vertexDescription->AddBinding(BINDING_1, sizeof(InstanceDataGPU), VK_VERTEX_INPUT_RATE_INSTANCE);
		vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());	// Location 0 : InInstanceWorld
		vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());	// Location 1 : InInstanceWorld
		vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());	// Location 2 : InInstanceWorld
		vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());	// Location 3 : InInstanceWorld

		effectDesc.shaderDesc.vertexShaderPath = "data/shaders/shadowmap/shadowmap_instancing.vert";
		effectDesc.shaderDesc.fragmentShaderPath = "data/shaders/shadowmap/shadowmap.frag";
		effectDesc.pipelineDesc.rasterizationState.cullMode = VK_CULL_MODE_NONE;
		effectDesc.pipelineDesc.OverrideVertexInput(vertexDescription);
		mEffectInstanced = Vk::gEffectManager().AddEffect<Vk::Effect>(device, mRenderPass.get(), effectDesc);

		mCascadeTransforms.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mEffect->BindUniformBuffer("UBO_cascadeTransforms", mCascadeTransforms);
		mEffectInstanced->BindUniformBuffer("UBO_cascadeTransforms", mCascadeTransforms);

      mQueryPool = std::make_shared<Vk::QueryPoolTimestamp>(device);

		const uint32_t size = 240;
		/*gScreenQuadUi().AddQuad(4 * (size + 10) + 10, height - (size + 10), size, size, depthColorImage->GetLayerView(0), renderTarget->GetSampler());
		gScreenQuadUi().AddQuad(5 * (size + 10) + 10, height - (size + 10), size, size, depthColorImage->GetLayerView(1), renderTarget->GetSampler());
		gScreenQuadUi().AddQuad(6 * (size + 10) + 10, height - (size + 10), size, size, depthColorImage->GetLayerView(2), renderTarget->GetSampler());
		gScreenQuadUi().AddQuad(4 * (size + 10) + 10, height - 2 * (size + 10), size, size, depthColorImage->GetLayerView(3), renderTarget->GetSampler());*/
	}

	ShadowJob::~ShadowJob()
	{
	}

	void ShadowJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
	}

	void ShadowJob::Render(const JobInput& jobInput)
	{
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			mCascadeTransforms.data.viewProjection[i] = jobInput.sceneInfo.cascades[i].viewProjMatrix;
		}

		mCascadeTransforms.UpdateMemory();

      mCommandBuffer->Begin();
      Vk::DebugLabel::BeginRegion(mCommandBuffer->GetVkHandle(), "Cascade pass", glm::vec4(1.0, 1.0, 0.0, 1.0));
      mQueryPool->Reset(mCommandBuffer.get());
      mQueryPool->Begin(mCommandBuffer.get());

		for (uint32_t cascadeIndex = 0; cascadeIndex < SHADOW_MAP_CASCADE_COUNT; cascadeIndex++)
		{
			// Begin the renderpass with the framebuffer attachments connected to the current cascade layer
         VkRenderPassBeginInfo renderPassBeginInfo = {};
         renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
         renderPassBeginInfo.renderPass = mRenderPass->GetVkHandle();
         renderPassBeginInfo.renderArea.extent.width = SHADOWMAP_DIMENSION;
         renderPassBeginInfo.renderArea.extent.height = SHADOWMAP_DIMENSION;
         renderPassBeginInfo.clearValueCount = (uint32_t)mClearValues.size();
         renderPassBeginInfo.pClearValues = mClearValues.data();
         renderPassBeginInfo.framebuffer = mFrameBuffers[cascadeIndex]->GetFrameBuffer(0);

         mCommandBuffer->CmdBeginRenderPass(&renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
         mCommandBuffer->CmdSetViewPort((float)SHADOWMAP_DIMENSION, (float)SHADOWMAP_DIMENSION);
         mCommandBuffer->CmdSetScissor(SHADOWMAP_DIMENSION, SHADOWMAP_DIMENSION);

			if (IsEnabled())
			{
				/* Render instanced assets */
            mCommandBuffer->CmdBindPipeline(mEffectInstanced->GetPipeline());

				for (uint32_t i = 0; i < jobInput.sceneInfo.instanceGroups.size(); i++)
				{
					SharedPtr<InstanceGroup> instanceGroup = jobInput.sceneInfo.instanceGroups[i];

					// Skip if instance group does not cast shadows
					if (!instanceGroup->IsCastingShadows())
						continue;

					Vk::Buffer* instanceBuffer = instanceGroup->GetBuffer();
					Vk::StaticModel* model = instanceGroup->GetModel();

					if (instanceBuffer != nullptr && model != nullptr)
					{
						for (Vk::Mesh* mesh : model->mMeshes)
						{
							CascadePushConst pushConst(glm::mat4(), cascadeIndex);
							mCommandBuffer->CmdPushConstants(mEffectInstanced->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(CascadePushConst), &pushConst);

							VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptorSet();
							VkDescriptorSet descriptorSets[2] = { mEffectInstanced->GetDescriptorSet(0).GetVkHandle(), textureDescriptorSet };
							mCommandBuffer->CmdBindDescriptorSet(mEffectInstanced->GetPipelineInterface(), 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);

							mCommandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
							mCommandBuffer->CmdBindVertexBuffer(1, 1, instanceBuffer);
							mCommandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
							mCommandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), instanceGroup->GetNumInstances(), 0, 0, 0);
						}
					}
				}

				mCommandBuffer->CmdBindPipeline(mEffect->GetPipeline());

				/* Render all renderables */
				for (auto& renderable : jobInput.sceneInfo.renderables)
				{
					if (!renderable->IsVisible() || !renderable->HasRenderFlags(RENDER_FLAG_CAST_SHADOW))
						continue;

					Vk::StaticModel* model = renderable->GetModel();

					for (Vk::Mesh* mesh : model->mMeshes)
					{
						CascadePushConst pushConst(renderable->GetTransform().GetWorldMatrix(), cascadeIndex);
						mCommandBuffer->CmdPushConstants(mEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(CascadePushConst), &pushConst);

						VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptorSet();
						VkDescriptorSet descriptorSets[2] = { mEffect->GetDescriptorSet(0).GetVkHandle(), textureDescriptorSet };
						mCommandBuffer->CmdBindDescriptorSet(mEffect->GetPipelineInterface(), 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);

						mCommandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
						mCommandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
						mCommandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
					}
				}
			}

			mCommandBuffer->CmdEndRenderPass();
		}

		mQueryPool->End(mCommandBuffer.get());
		Vk::DebugLabel::EndRegion(mCommandBuffer->GetVkHandle());

		mCommandBuffer->Submit(GetWaitSemahore(), GetCompletedSemahore());

		if (gProfiler().IsEnabled())
			gProfiler().AddProfilerTask("Cascade pass: ", mQueryPool->GetElapsedTime(), glm::vec4(1.0, 1.0, 0.0, 1.0));
	}
}
