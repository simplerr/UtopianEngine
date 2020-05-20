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
		mDepthImage = std::make_shared<Vk::ImageDepth>(device, SHADOWMAP_DIMENSION, SHADOWMAP_DIMENSION, VK_FORMAT_D32_SFLOAT_S8_UINT);

		mRenderTarget = std::make_shared<Vk::RenderTarget>(device, SHADOWMAP_DIMENSION, SHADOWMAP_DIMENSION);
		mRenderTarget->AddColorAttachment(depthColorImage->GetLayerView(0), depthColorImage->GetFormat());
		mRenderTarget->AddWriteOnlyDepthAttachment(mDepthImage);
		mRenderTarget->SetClearColor(1, 1, 1, 1);
		mRenderTarget->Create();

		// Create multiple framebuffers with attachments to the individual array layers in the depth buffer image
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			SharedPtr<Vk::FrameBuffers> frameBuffer = std::make_shared<Vk::FrameBuffers>(device);
			frameBuffer->AddAttachmentImage(depthColorImage->GetLayerView(i));
			frameBuffer->AddAttachmentImage(mDepthImage.get());
			frameBuffer->Create(mRenderTarget->GetRenderPass(), SHADOWMAP_DIMENSION, SHADOWMAP_DIMENSION);
			mFrameBuffers.push_back(frameBuffer);
		}

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/shadowmap/shadowmap.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/shadowmap/shadowmap.frag";

		mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(device, mRenderTarget->GetRenderPass(), shaderCreateInfo);
		mEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mEffect->CreatePipeline();

		shaderCreateInfo.vertexShaderPath = "data/shaders/shadowmap/shadowmap_instancing.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/shadowmap/shadowmap.frag";
		mEffectInstanced = Vk::gEffectManager().AddEffect<Vk::Effect>(device, mRenderTarget->GetRenderPass(), shaderCreateInfo);

		mEffectInstanced->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_NONE;

		SharedPtr<Vk::VertexDescription> vertexDescription = std::make_shared<Vk::VertexDescription>(Vk::Vertex::GetDescription());
		vertexDescription->AddBinding(BINDING_1, sizeof(InstanceData), VK_VERTEX_INPUT_RATE_INSTANCE);
		vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());	// Location 0 : InInstanceWorld
		vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());	// Location 1 : InInstanceWorld
		vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());	// Location 2 : InInstanceWorld
		vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());	// Location 3 : InInstanceWorld
		mEffectInstanced->GetPipeline()->OverrideVertexInput(vertexDescription);

		mEffectInstanced->CreatePipeline();

		mCascadeTransforms.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mEffect->BindUniformBuffer("UBO_cascadeTransforms", mCascadeTransforms);
		mEffectInstanced->BindUniformBuffer("UBO_cascadeTransforms", mCascadeTransforms);

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

		Vk::CommandBuffer* commandBuffer = mRenderTarget->GetCommandBuffer();
		commandBuffer->Begin();

		for (uint32_t cascadeIndex = 0; cascadeIndex < SHADOW_MAP_CASCADE_COUNT; cascadeIndex++)
		{
			// Begin the renderpass with the framebuffer attachments connected to the current cascade layer
			mRenderTarget->Begin(mFrameBuffers[cascadeIndex]->GetFrameBuffer(0), "Cascade pass", glm::vec4(1.0, 1.0, 0.0, 1.0));

			if (IsEnabled())
			{
				/* Render instanced assets */
				commandBuffer->CmdBindPipeline(mEffectInstanced->GetPipeline());

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
							commandBuffer->CmdPushConstants(mEffectInstanced->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(CascadePushConst), &pushConst);

							VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptorSet();
							VkDescriptorSet descriptorSets[2] = { mEffectInstanced->GetDescriptorSet(0).GetVkHandle(), textureDescriptorSet };
							commandBuffer->CmdBindDescriptorSet(mEffectInstanced->GetPipelineInterface(), 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);

							commandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
							commandBuffer->CmdBindVertexBuffer(1, 1, instanceBuffer);
							commandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
							commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), instanceGroup->GetNumInstances(), 0, 0, 0);
						}
					}
				}

				commandBuffer->CmdBindPipeline(mEffect->GetPipeline());

				/* Render all renderables */
				for (auto& renderable : jobInput.sceneInfo.renderables)
				{
					if (!renderable->IsVisible() || ((renderable->GetRenderFlags() & RENDER_FLAG_DEFERRED) != RENDER_FLAG_DEFERRED))
						continue;

					Vk::StaticModel* model = renderable->GetModel();

					for (Vk::Mesh* mesh : model->mMeshes)
					{
						CascadePushConst pushConst(renderable->GetTransform().GetWorldMatrix(), cascadeIndex);
						commandBuffer->CmdPushConstants(mEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(CascadePushConst), &pushConst);

						VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptorSet();
						VkDescriptorSet descriptorSets[2] = { mEffect->GetDescriptorSet(0).GetVkHandle(), textureDescriptorSet };
						commandBuffer->CmdBindDescriptorSet(mEffect->GetPipelineInterface(), 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);

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
