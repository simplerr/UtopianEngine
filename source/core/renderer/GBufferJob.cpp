#include "core/renderer/GBufferJob.h"
#include "core/renderer/GBufferTerrainJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "vulkan/Debug.h"
#include "vulkan/handles/Queue.h"

namespace Utopian
{
	GBufferJob::GBufferJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		
	}

	GBufferJob::~GBufferJob()
	{
	}

	void GBufferJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		GBufferTerrainJob* gbufferTerrainJob = static_cast<GBufferTerrainJob*>(jobs[JobGraph::GBUFFER_TERRAIN_INDEX]);

		renderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
		renderTarget->AddReadWriteColorAttachment(gbuffer.positionImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->AddReadWriteColorAttachment(gbuffer.normalImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->AddReadWriteColorAttachment(gbuffer.albedoImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->AddReadWriteColorAttachment(gbuffer.normalViewImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		renderTarget->AddReadWriteDepthAttachment(gbuffer.depthImage);
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		// Todo: Implement a better way for multiple pipelines in the same Effect
		mGBufferEffect = Vk::gEffectManager().AddEffect<Vk::GBufferEffect>(mDevice, renderTarget->GetRenderPass());
		mGBufferEffectWireframe = Vk::gEffectManager().AddEffect<Vk::GBufferEffect>(mDevice, renderTarget->GetRenderPass());
		mGBufferEffectWireframe->GetPipeline()->rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/gbuffer/gbuffer_instancing_animation.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/gbuffer/gbuffer.frag";

		mInstancedAnimationEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, renderTarget->GetRenderPass(), shaderCreateInfo);

		shaderCreateInfo.vertexShaderPath = "data/shaders/gbuffer/gbuffer_instancing.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/gbuffer/gbuffer.frag";

		mGBufferEffectInstanced = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, renderTarget->GetRenderPass(), shaderCreateInfo);

		mGBufferEffectInstanced->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_NONE;

		//SharedPtr<Vk::VertexDescription> vertexDescription = std::make_shared<Vk::VertexDescription>();
		SharedPtr<Vk::VertexDescription> vertexDescription = std::make_shared<Vk::VertexDescription>(Vk::Vertex::GetDescription());
		vertexDescription->AddBinding(BINDING_1, sizeof(InstanceData), VK_VERTEX_INPUT_RATE_INSTANCE);
		vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());	// Location 0 : InInstanceWorld
		vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());	// Location 1 : InInstanceWorld
		vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());	// Location 2 : InInstanceWorld
		vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());	// Location 3 : InInstanceWorld
		mGBufferEffectInstanced->GetPipeline()->OverrideVertexInput(vertexDescription);
		mInstancedAnimationEffect->GetPipeline()->OverrideVertexInput(vertexDescription);

		mGBufferEffectInstanced->CreatePipeline();
		mInstancedAnimationEffect->CreatePipeline();
		mGBufferEffectWireframe->CreatePipeline();

		viewProjectionBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mInstancedAnimationEffect->BindUniformBuffer("UBO_viewProjection", &viewProjectionBlock);
		mGBufferEffectInstanced->BindUniformBuffer("UBO_viewProjection", &viewProjectionBlock);

		settingsBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mGBufferEffect->BindUniformBuffer("UBO_settings", &settingsBlock);
		mGBufferEffectWireframe->BindUniformBuffer("UBO_settings", &settingsBlock);
		mInstancedAnimationEffect->BindUniformBuffer("UBO_settings", &settingsBlock);
		mGBufferEffectInstanced->BindUniformBuffer("UBO_settings", &settingsBlock);

		animationParametersBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mInstancedAnimationEffect->BindUniformBuffer("UBO_animationParameters", &animationParametersBlock);

		mWindmapTexture = Vk::gTextureLoader().LoadTexture("data/textures/windmap.jpg");
		mInstancedAnimationEffect->BindCombinedImage("windmapSampler", mWindmapTexture->GetTextureDescriptorInfo());
	}

	void GBufferJob::Render(const JobInput& jobInput)
	{
		mGBufferEffect->SetCameraData(jobInput.sceneInfo.viewMatrix, jobInput.sceneInfo.projectionMatrix);
		mGBufferEffectWireframe->SetCameraData(jobInput.sceneInfo.viewMatrix, jobInput.sceneInfo.projectionMatrix);
		viewProjectionBlock.data.view = jobInput.sceneInfo.viewMatrix;
		viewProjectionBlock.data.projection = jobInput.sceneInfo.projectionMatrix;
		viewProjectionBlock.UpdateMemory();

		settingsBlock.data.normalMapping = jobInput.renderingSettings.normalMapping;
		settingsBlock.UpdateMemory();

		animationParametersBlock.data.time = gTimer().GetTime();
		animationParametersBlock.data.terrainSize = jobInput.sceneInfo.terrain->GetTerrainSize();
		animationParametersBlock.data.strength = jobInput.renderingSettings.windStrength;
		animationParametersBlock.data.frequency = jobInput.renderingSettings.windFrequency;
		animationParametersBlock.UpdateMemory();

		renderTarget->Begin("G-buffer pass");
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		/* Render instanced assets */
		for (uint32_t i = 0; i < jobInput.sceneInfo.instanceGroups.size(); i++)
		{
			SharedPtr<InstanceGroup> instanceGroup = jobInput.sceneInfo.instanceGroups[i];
			Vk::Buffer* instanceBuffer = instanceGroup->GetBuffer();
			Vk::StaticModel* model = instanceGroup->GetModel();

			if (instanceBuffer != nullptr && model != nullptr)
			{
				SharedPtr<Vk::Effect> effect = nullptr;
				if (!instanceGroup->IsAnimated())
					effect = mGBufferEffectInstanced;
				else
					effect = mInstancedAnimationEffect;

				commandBuffer->CmdBindPipeline(effect->GetPipeline());

				for (Vk::Mesh* mesh : model->mMeshes)
				{
					VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptorSet();
					VkDescriptorSet descriptorSets[2] = { effect->GetDescriptorSet(0).GetVkHandle(), textureDescriptorSet };

					commandBuffer->CmdBindDescriptorSet(effect->GetPipelineInterface(), 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);

					// Note: Move out from if
					commandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
					commandBuffer->CmdBindVertexBuffer(1, 1, instanceBuffer);
					commandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
					commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), instanceGroup->GetNumInstances(), 0, 0, 0);
				}
			}
		}

		/* Render all renderables */
		for (auto& renderable : jobInput.sceneInfo.renderables)
		{
			//if (!renderable->IsVisible() || ((renderable->GetRenderFlags() & RENDER_FLAG_DEFERRED) != RENDER_FLAG_DEFERRED))
			if (!renderable->IsVisible() || !(renderable->HasRenderFlags(RENDER_FLAG_DEFERRED) || renderable->HasRenderFlags(RENDER_FLAG_WIREFRAME)))
				continue;

			Vk::Effect* effect = mGBufferEffect.get();
			if (renderable->HasRenderFlags(RENDER_FLAG_WIREFRAME))
				effect = mGBufferEffectWireframe.get();

			Vk::StaticModel* model = renderable->GetModel();

			for (Vk::Mesh* mesh : model->mMeshes)
			{
				// Push the world matrix constant
				Vk::PushConstantBlock pushConsts(renderable->GetTransform().GetWorldMatrix(), renderable->GetColor());

				VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptorSet();
				VkDescriptorSet descriptorSets[2] = { effect->GetDescriptorSet(0).GetVkHandle(), textureDescriptorSet };

				commandBuffer->CmdBindPipeline(effect->GetPipeline());
				commandBuffer->CmdBindDescriptorSet(effect->GetPipelineInterface(), 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);
				commandBuffer->CmdPushConstants(effect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConsts), &pushConsts);

				// Note: Move out from if
				commandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
				commandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
				commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
			}
		}	
		
		Vk::DebugMarker::EndRegion(commandBuffer->GetVkHandle());

		renderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
	}
}