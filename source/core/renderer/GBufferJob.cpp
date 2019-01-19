#include "core/renderer/GBufferJob.h"
#include "core/renderer/CommonJobIncludes.h"

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
		//mGBufferEffectWireframe->GetPipeline()->rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		mGBufferEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_NONE;

		mGBufferEffectTerrain = Vk::gEffectManager().AddEffect<Vk::Effect>(mRenderer->GetDevice(),
																		   renderTarget->GetRenderPass(),
																		   "data/shaders/gbuffer/gbuffer.vert",
																		   "data/shaders/gbuffer/gbuffer_terrain.frag");

		mGBufferEffectInstanced = Vk::gEffectManager().AddEffect<Vk::Effect>(mRenderer->GetDevice(),
																		     renderTarget->GetRenderPass(),
																		   	 "data/shaders/gbuffer/gbuffer_instancing.vert",
																		     "data/shaders/gbuffer/gbuffer.frag");

		mGBufferEffectInstanced->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_NONE;

		//SharedPtr<Vk::VertexDescription> vertexDescription = std::make_shared<Vk::VertexDescription>();
		SharedPtr<Vk::VertexDescription> vertexDescription = std::make_shared<Vk::VertexDescription>(Vk::Vertex::GetDescription());
		vertexDescription->AddBinding(BINDING_1, sizeof(InstanceData), VK_VERTEX_INPUT_RATE_INSTANCE);
		vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());	// Location 0 : InInstanceWorld
		vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());	// Location 1 : InInstanceWorld
		vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());	// Location 2 : InInstanceWorld
		vertexDescription->AddAttribute(BINDING_1, Vk::Vec4Attribute());	// Location 3 : InInstanceWorld
		mGBufferEffectInstanced->GetPipeline()->OverrideVertexInput(vertexDescription);

		mGBufferEffectInstanced->CreatePipeline();
		mGBufferEffectTerrain->CreatePipeline();
		mGBufferEffect->CreatePipeline();
		mGBufferEffectWireframe->CreatePipeline();

		viewProjectionBlock.Create(renderer->GetDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mGBufferEffectTerrain->BindUniformBuffer("UBO_viewProjection", &viewProjectionBlock);
		mGBufferEffectInstanced->BindUniformBuffer("UBO_viewProjection", &viewProjectionBlock);

		settingsBlock.Create(renderer->GetDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mGBufferEffect->BindUniformBuffer("UBO_settings", &settingsBlock);
		mGBufferEffectTerrain->BindUniformBuffer("UBO_settings", &settingsBlock);
		mGBufferEffectInstanced->BindUniformBuffer("UBO_settings", &settingsBlock);

		// Bind the different terrain textures
		sampler = std::make_shared<Vk::Sampler>(mRenderer->GetDevice(), false);
		sampler->Create();

		Vk::Texture* texture = Vk::gTextureLoader().LoadTexture("data/textures/ground/grass2.png");
		Vk::Texture* texture2 = Vk::gTextureLoader().LoadTexture("data/textures/ground/rock.png");
		Vk::Texture* texture3 = Vk::gTextureLoader().LoadTexture("data/textures/ground/snow.png");
		Vk::TextureArray textureArray;
		textureArray.AddTexture(texture->imageView, sampler.get());
		textureArray.AddTexture(texture2->imageView, sampler.get());
		textureArray.AddTexture(texture3->imageView, sampler.get());

		mGBufferEffectTerrain->BindCombinedImage("textureSampler", &textureArray);

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
		viewProjectionBlock.data.view = jobInput.sceneInfo.viewMatrix;
		viewProjectionBlock.data.projection = jobInput.sceneInfo.projectionMatrix;
		viewProjectionBlock.UpdateMemory();

		settingsBlock.data.normalMapping = jobInput.renderingSettings.normalMapping;
		settingsBlock.UpdateMemory();

		renderTarget->Begin();
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		/* Render instanced assets */
		commandBuffer->CmdBindPipeline(mGBufferEffectInstanced->GetPipeline());
		for (uint32_t i = 0; i < jobInput.sceneInfo.instanceGroups.size(); i++)
		{
			SharedPtr<InstanceGroup> instanceGroup = jobInput.sceneInfo.instanceGroups[i];
			Vk::Buffer* instanceBuffer = instanceGroup->GetBuffer();
			Vk::StaticModel* model = instanceGroup->GetModel();

			if (instanceBuffer != nullptr && model != nullptr)
			{
				for (Vk::Mesh* mesh : model->mMeshes)
				{
					VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptorSet();
					VkDescriptorSet descriptorSets[2] = { mGBufferEffectInstanced->GetDescriptorSet(0).descriptorSet, textureDescriptorSet };

					commandBuffer->CmdBindDescriptorSet(mGBufferEffectInstanced->GetPipelineInterface(), 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);

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
				Vk::PushConstantBlock pushConsts(renderable->GetTransform().GetWorldMatrix(), renderable->GetColor(), renderable->GetTextureTiling());

				// Todo: Note: This is a temporary workaround
				if (!renderable->HasRenderFlags(RENDER_FLAG_TERRAIN))
				{
					VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptorSet();
					VkDescriptorSet descriptorSets[2] = { effect->GetDescriptorSet(0).descriptorSet, textureDescriptorSet };

					commandBuffer->CmdBindPipeline(effect->GetPipeline());
					commandBuffer->CmdBindDescriptorSet(effect->GetPipelineInterface(), 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);
					commandBuffer->CmdPushConstants(effect->GetPipelineInterface(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConsts), &pushConsts);

					// Note: Move out from if
					commandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
					commandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
					commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
				}
				else
				{
					commandBuffer->CmdBindPipeline(mGBufferEffectTerrain->GetPipeline());
					mGBufferEffectTerrain->BindDescriptorSets(commandBuffer);
					commandBuffer->CmdPushConstants(mGBufferEffectTerrain->GetPipelineInterface(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConsts), &pushConsts);

					// Note: Move out from else
					commandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
					commandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
					commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
				}

			}
		}

		renderTarget->End(renderer->GetQueue());
	}
}