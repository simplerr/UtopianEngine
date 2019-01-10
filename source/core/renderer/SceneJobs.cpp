#include "core/renderer/SceneJobs.h"
#include "core/renderer/Renderable.h"
#include "core/renderer/RenderingManager.h"
#include "vulkan/Renderer.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/RenderTarget.h"
#include "vulkan/BasicRenderTarget.h"
#include "vulkan/StaticModel.h"
#include "vulkan/handles/Image.h"
#include "vulkan/handles/FrameBuffers.h"
#include "vulkan/handles/Sampler.h"
#include "vulkan/handles/Pipeline.h"
#include "vulkan/handles/RenderPass.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/ScreenGui.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/handles/Texture.h"
#include "vulkan/EffectManager.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/CubeMapTexture.h"
#include "core/renderer/Light.h"
#include "core/terrain/PerlinTerrain.h"
#include "utility/Timer.h"
#include <random>

namespace Utopian
{
	static void calculateLightViewProj(Light* light, glm::mat4& view, glm::mat4& projection, uint32_t width, uint32_t height)
	{
		const float farPlane = 20000.0f;
		const float size = 2000.0f;
		view = glm::lookAt(-light->GetPosition(), glm::vec3(0.0f), glm::vec3(0, 1, 0));
		//projection = glm::perspective<float>(glm::radians(45.0f), (float)width / (float)height, 1.0f, farPlane);
		projection = glm::ortho(size, 0.0f, 0.0f, size, -farPlane, farPlane);
	}

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

		renderTarget->Begin();
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		/* Render the terrain */
		std::map<BlockKey, SharedPtr<Block2>> blockList = jobInput.sceneInfo.terrain->GetBlocks();

		//for (auto& iter : blockList)
		//{
		//	Vk::Effect* effect = mGBufferEffect.get();
		//	bool wireframeTerrain = false;
		//	if (wireframeTerrain)
		//		effect = mGBufferEffectWireframe.get();

		//	SharedPtr<Vk::Mesh> blockMesh = iter.second->renderable;
		//	glm::vec3 position = iter.second->position;
		//	glm::vec3 color = iter.second->color;
		//	glm::mat4 world = glm::translate(glm::mat4(), position);

		//	// Push the world matrix constant
		//	Vk::PushConstantBlock pushConsts(world, glm::vec4(color, 1.0f));

		//	// Todo: Note: This is a temporary workaround
		//	VkDescriptorSet textureDescriptorSet = blockMesh->GetTextureDescriptor();
		//	VkDescriptorSet descriptorSets[2] = { effect->GetDescriptorSet(0).descriptorSet, textureDescriptorSet };

		//	commandBuffer->CmdBindPipeline(effect->GetPipeline());
		//	commandBuffer->CmdBindDescriptorSet(effect->GetPipelineInterface(), 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);
		//	commandBuffer->CmdPushConstants(effect->GetPipelineInterface(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConsts), &pushConsts);
		//	commandBuffer->CmdBindVertexBuffer(0, 1, blockMesh->GetVertxBuffer());
		//	commandBuffer->CmdBindIndexBuffer(blockMesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
		//	commandBuffer->CmdDrawIndexed(blockMesh->GetNumIndices(), 1, 0, 0, 0);
		//}

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

	ShadowJob::ShadowJob(Vk::Renderer* renderer, uint32_t width, uint32_t height)
		: BaseJob(renderer, width, height)
	{
		depthColorImage = std::make_shared<Vk::ImageColor>(renderer->GetDevice(), SHADOWMAP_DIMENSION, SHADOWMAP_DIMENSION, VK_FORMAT_R32_SFLOAT, 4);
		depthImage = std::make_shared<Vk::ImageDepth>(renderer->GetDevice(), SHADOWMAP_DIMENSION, SHADOWMAP_DIMENSION, VK_FORMAT_D32_SFLOAT_S8_UINT);

		renderTarget = std::make_shared<Vk::RenderTarget>(renderer->GetDevice(), renderer->GetCommandPool(), SHADOWMAP_DIMENSION, SHADOWMAP_DIMENSION);
		renderTarget->AddColorAttachment(depthColorImage->GetLayerView(0), depthColorImage->GetFormat());
		renderTarget->AddDepthAttachment(depthImage);
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		// Create multiple framebuffers with attachments to the individual array layers in the depth buffer image
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			SharedPtr<Vk::FrameBuffers> frameBuffer = std::make_shared<Vk::FrameBuffers>(renderer->GetDevice());
			frameBuffer->AddAttachmentImage(depthColorImage->GetLayerView(i));
			frameBuffer->AddAttachmentImage(depthImage.get());
			frameBuffer->Create(renderTarget->GetRenderPass(), SHADOWMAP_DIMENSION, SHADOWMAP_DIMENSION);
			mFrameBuffers.push_back(frameBuffer);
		}

 		effect = Vk::gEffectManager().AddEffect<Vk::Effect>(renderer->GetDevice(),
															renderTarget->GetRenderPass(),
															"data/shaders/shadowmap/shadowmap.vert",
															"data/shaders/shadowmap/shadowmap.frag");
		effect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		effect->CreatePipeline();

		effectInstanced = Vk::gEffectManager().AddEffect<Vk::Effect>(renderer->GetDevice(),
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

		cascadeTransforms.Create(renderer->GetDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		effect->BindUniformBuffer("UBO_cascadeTransforms", &cascadeTransforms);
		effectInstanced->BindUniformBuffer("UBO_cascadeTransforms", &cascadeTransforms);

		const uint32_t size = 240;
		renderer->AddScreenQuad(4 * (size + 10) + 10, height - (size + 10), size, size, depthColorImage->GetLayerView(0), renderTarget->GetSampler());
		renderer->AddScreenQuad(5 * (size + 10) + 10, height - (size + 10), size, size, depthColorImage->GetLayerView(1), renderTarget->GetSampler());
		renderer->AddScreenQuad(6 * (size + 10) + 10, height - (size + 10), size, size, depthColorImage->GetLayerView(2), renderTarget->GetSampler());
		renderer->AddScreenQuad(4 * (size + 10) + 10, height - 2 * (size + 10), size, size, depthColorImage->GetLayerView(3), renderTarget->GetSampler());
	}

	ShadowJob::~ShadowJob()
	{
	}

	void ShadowJob::Init(const std::vector<BaseJob*>& jobs)
	{
		
	}

	void ShadowJob::Render(Vk::Renderer* renderer, const JobInput& jobInput)
	{
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			cascadeTransforms.data.viewProjection[i] = jobInput.sceneInfo.cascades[i].viewProjMatrix;
		}

		cascadeTransforms.UpdateMemory();

		for (uint32_t cascadeIndex = 0; cascadeIndex < SHADOW_MAP_CASCADE_COUNT; cascadeIndex++)
		{
			// Begin the renderpass with the framebuffer attachments connected to the current cascade layer
			renderTarget->Begin(mFrameBuffers[cascadeIndex]->GetFrameBuffer(0));
			Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

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
						VkDescriptorSet descriptorSets[2] = { effectInstanced->GetDescriptorSet(0).descriptorSet, textureDescriptorSet };
						commandBuffer->CmdBindDescriptorSet(effectInstanced->GetPipelineInterface(), 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);

						commandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
						commandBuffer->CmdBindVertexBuffer(1, 1, instanceBuffer);
						commandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
						commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), instanceGroup->GetNumInstances(), 0, 0, 0);
					}
				}
			}

			// Todo: Should this be moved to the effect instead?
			commandBuffer->CmdBindPipeline(effect->GetPipeline());
			effect->BindDescriptorSets(commandBuffer);

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
					VkDescriptorSet descriptorSets[2] = { effect->GetDescriptorSet(0).descriptorSet, textureDescriptorSet };
					commandBuffer->CmdBindDescriptorSet(effect->GetPipelineInterface(), 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);

					commandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
					commandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
					commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
				}
			}

			renderTarget->End(renderer->GetQueue());
		}
	}

	DeferredJob::DeferredJob(Vk::Renderer* renderer, uint32_t width, uint32_t height)
		: BaseJob(renderer, width, height)
	{
		renderTarget = std::make_shared<Vk::BasicRenderTarget>(renderer->GetDevice(), renderer->GetCommandPool(), width, height, VK_FORMAT_R8G8B8A8_UNORM);
		effect = Vk::gEffectManager().AddEffect<Vk::DeferredEffect>(renderer->GetDevice(), renderTarget->GetRenderPass());

		mScreenQuad = renderer->AddScreenQuad(0u, 0u, width, height, renderTarget->GetColorImage(), renderTarget->GetSampler(), 1u);

		// Create sampler that returns 1.0 when sampling outside the depth image
		depthSampler = std::make_shared<Vk::Sampler>(renderer->GetDevice(), false);
		depthSampler->createInfo.anisotropyEnable = VK_FALSE; // Anistropy filter causes artifacts at the edge between cascades
		depthSampler->createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		depthSampler->createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		depthSampler->createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		depthSampler->createInfo.borderColor = VkBorderColor::VK_BORDER_COLOR_INT_OPAQUE_WHITE;
		depthSampler->Create();
	}

	DeferredJob::~DeferredJob()
	{
	}

	void DeferredJob::Init(const std::vector<BaseJob*>& jobs)
	{
		GBufferJob* gbufferJob = static_cast<GBufferJob*>(jobs[RenderingManager::GBUFFER_INDEX]);
		BlurJob* blurJob = static_cast<BlurJob*>(jobs[RenderingManager::BLUR_INDEX]);
		ShadowJob* shadowJob = static_cast<ShadowJob*>(jobs[RenderingManager::SHADOW_INDEX]);
		effect->BindImages(gbufferJob->positionImage.get(),
						   gbufferJob->normalImage.get(),
						   gbufferJob->albedoImage.get(),
						   blurJob->blurImage.get(),
						   gbufferJob->renderTarget->GetSampler());

		effect->BindCombinedImage("shadowSampler", shadowJob->depthColorImage.get(), depthSampler.get());
	}

	void DeferredJob::Render(Vk::Renderer* renderer, const JobInput& jobInput)
	{
		mScreenQuad->SetVisible(jobInput.renderingSettings.deferredPipeline);

		effect->SetSettingsData(jobInput.renderingSettings);
		effect->SetEyePos(glm::vec4(jobInput.sceneInfo.eyePos, 1.0f));
		effect->SetLightArray(jobInput.sceneInfo.lights);

		// Note: Todo: Temporary
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			effect->cascade_ubo.data.cascadeSplits[i] = jobInput.sceneInfo.cascades[i].splitDepth;
			effect->cascade_ubo.data.cascadeViewProjMat[i] = jobInput.sceneInfo.cascades[i].viewProjMatrix;
		}

		// Note: This should probably be moved. We need the fragment position in view space
		// when comparing it's Z value to find out which shadow map cascade it should sample from.
		effect->cascade_ubo.data.cameraViewMat = jobInput.sceneInfo.viewMatrix;
		effect->cascade_ubo.UpdateMemory();

		// End of temporary

		Light* directionalLight = jobInput.sceneInfo.directionalLight;

		glm::mat4 lightView = glm::mat4();
		glm::mat4 lightProjection = glm::mat4();
		calculateLightViewProj(directionalLight, lightView, lightProjection, mWidth, mHeight);
		effect->SetLightTransform(lightProjection * lightView);

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
		GBufferJob* gbufferRenderer = static_cast<GBufferJob*>(renderers[RenderingManager::GBUFFER_INDEX]);
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
		SSAOJob* ssaoJob = static_cast<SSAOJob*>(renderers[RenderingManager::SSAO_INDEX]);
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
		DeferredJob* deferredJob = static_cast<DeferredJob*>(renderers[RenderingManager::DEFERRED_INDEX]);
		GBufferJob* gbufferJob = static_cast<GBufferJob*>(renderers[RenderingManager::GBUFFER_INDEX]);

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

		commandBuffer->CmdBindVertexBuffer(0, 1, mCubeModel->mMeshes[0]->GetVertxBuffer());
		commandBuffer->CmdBindIndexBuffer(mCubeModel->mMeshes[0]->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
		commandBuffer->CmdDrawIndexed(mCubeModel->GetNumIndices(), 1, 0, 0, 0);

		renderTarget->End(renderer->GetQueue());
	}

	SkydomeJob::SkydomeJob(Vk::Renderer* renderer, uint32_t width, uint32_t height)
		: BaseJob(renderer, width, height)
	{
		parameterBlock.data.inclination = 90.0f;
		parameterBlock.data.azimuth = 0.0f;
		sunAzimuth = 0.0f;
	}

	SkydomeJob::~SkydomeJob()
	{
	}

	void SkydomeJob::Init(const std::vector<BaseJob*>& renderers)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(renderers[RenderingManager::DEFERRED_INDEX]);
		GBufferJob* gbufferJob = static_cast<GBufferJob*>(renderers[RenderingManager::GBUFFER_INDEX]);

		renderTarget = std::make_shared<Vk::RenderTarget>(mRenderer->GetDevice(), mRenderer->GetCommandPool(), mWidth, mHeight);
		renderTarget->AddColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ATTACHMENT_LOAD_OP_LOAD);
		renderTarget->AddDepthAttachment(gbufferJob->depthImage);
		// Todo: Investigate why this does not work
		renderTarget->GetRenderPass()->attachments[Vk::RenderPassAttachment::DEPTH_ATTACHMENT].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		effect = Vk::gEffectManager().AddEffect<Vk::Effect>(mRenderer->GetDevice(),
															renderTarget->GetRenderPass(),
															"data/shaders/skydome/skydome.vert",
															"data/shaders/skydome/skydome.frag");

		effect->GetPipeline()->depthStencilState.depthWriteEnable = VK_FALSE;
		effect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		//effect->GetPipeline()->rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		effect->CreatePipeline();

		viewProjectionBlock.Create(mRenderer->GetDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		parameterBlock.Create(mRenderer->GetDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		effect->BindUniformBuffer("UBO_viewProjection", &viewProjectionBlock);
		effect->BindUniformBuffer("UBO_parameters", &parameterBlock);

		mSkydomeModel = Vk::gModelLoader().LoadModel("data/models/sphere.obj");
	}

	void SkydomeJob::Render(Vk::Renderer* renderer, const JobInput& jobInput)
	{
		// Move sun
		sunAzimuth += Timer::Instance().GetTime() / 10000000 * jobInput.renderingSettings.sunSpeed;

		// Calculate light direction
		float sunInclination = glm::radians(jobInput.renderingSettings.sunInclination);

		glm::vec3 sunDir = glm::vec3(sin(sunInclination) * cos(sunAzimuth),
									 cos(sunInclination),
									 sin(sunInclination) * sin(sunAzimuth));

		// Note: Negation of Z
		jobInput.sceneInfo.directionalLight->SetDirection(glm::vec3(1, 1, -1) * sunDir);

		// Removes the translation components of the matrix to always keep the skydome at the same distance
		glm::mat4 world  = glm::rotate(glm::mat4(), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		viewProjectionBlock.data.view = glm::mat4(glm::mat3(jobInput.sceneInfo.viewMatrix));
		viewProjectionBlock.data.projection = jobInput.sceneInfo.projectionMatrix;
		viewProjectionBlock.data.world = glm::scale(world, glm::vec3(1000.0f));
		viewProjectionBlock.UpdateMemory();

		parameterBlock.data.sphereRadius = mSkydomeModel->GetBoundingBox().GetHeight() / 2.0f;
		parameterBlock.data.inclination = sunInclination;
		parameterBlock.data.azimuth = sunAzimuth;
		parameterBlock.data.time = Timer::Instance().GetTime();
		parameterBlock.data.sunSpeed = jobInput.renderingSettings.sunSpeed;
		parameterBlock.UpdateMemory();

		renderTarget->Begin();
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		// Todo: Should this be moved to the effect instead?
		commandBuffer->CmdBindPipeline(effect->GetPipeline());
		effect->BindDescriptorSets(commandBuffer);

		commandBuffer->CmdBindVertexBuffer(0, 1, mSkydomeModel->mMeshes[0]->GetVertxBuffer());
		commandBuffer->CmdBindIndexBuffer(mSkydomeModel->mMeshes[0]->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
		commandBuffer->CmdDrawIndexed(mSkydomeModel->GetNumIndices(), 1, 0, 0, 0);

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
		DeferredJob* deferredJob = static_cast<DeferredJob*>(renderers[RenderingManager::DEFERRED_INDEX]);
		GBufferJob* gbufferJob = static_cast<GBufferJob*>(renderers[RenderingManager::GBUFFER_INDEX]);

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
				glm::vec3 translation = glm::vec3(pos.x, boundingBox.GetMin().y + boundingBox.GetHeight()/2, pos.z);
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

		renderTarget->End(renderer->GetQueue());
	}

	GrassJob::GrassJob(Vk::Renderer* renderer, uint32_t width, uint32_t height)
		: BaseJob(renderer, width, height)
	{
		//effect->BindCombinedImage("textureSampler", todo)
	}

	GrassJob::~GrassJob()
	{
	}

	void GrassJob::Init(const std::vector<BaseJob*>& jobs)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[RenderingManager::DEFERRED_INDEX]);
		GBufferJob* gbufferJob = static_cast<GBufferJob*>(jobs[RenderingManager::GBUFFER_INDEX]);

		renderTarget = std::make_shared<Vk::RenderTarget>(mRenderer->GetDevice(), mRenderer->GetCommandPool(), mWidth, mHeight);
		renderTarget->AddColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ATTACHMENT_LOAD_OP_LOAD);
		renderTarget->AddDepthAttachment(gbufferJob->depthImage);
		renderTarget->GetRenderPass()->attachments[Vk::RenderPassAttachment::DEPTH_ATTACHMENT].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();
		
		effect = Vk::gEffectManager().AddEffect<Vk::Effect>(mRenderer->GetDevice(),
															renderTarget->GetRenderPass(),
			                                                "data/shaders/grass/grass.vert",
															"data/shaders/grass/grass.frag");

		SharedPtr<Vk::VertexDescription> vertexDescription = std::make_shared<Vk::VertexDescription>();

		vertexDescription->AddBinding(BINDING_0, sizeof(GrassInstance), VK_VERTEX_INPUT_RATE_INSTANCE);
		vertexDescription->AddAttribute(BINDING_0, Vk::Vec4Attribute());	// Location 0 : InstancePos
		vertexDescription->AddAttribute(BINDING_0, Vk::Vec3Attribute());	// Location 1 : Color
		vertexDescription->AddAttribute(BINDING_0, Vk::U32Attribute());		// Location 2 : InTexId

		effect->GetPipeline()->OverrideVertexInput(vertexDescription);
		effect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_NONE;
		effect->GetPipeline()->inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

		// Enable blending using alpha channel
		effect->GetPipeline()->blendAttachmentState[0].blendEnable = VK_TRUE;
		effect->GetPipeline()->blendAttachmentState[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		effect->GetPipeline()->blendAttachmentState[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		effect->GetPipeline()->blendAttachmentState[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		effect->GetPipeline()->blendAttachmentState[0].colorBlendOp = VK_BLEND_OP_ADD;
		effect->GetPipeline()->blendAttachmentState[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		effect->GetPipeline()->blendAttachmentState[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		effect->GetPipeline()->blendAttachmentState[0].alphaBlendOp = VK_BLEND_OP_ADD;
		effect->CreatePipeline();

		viewProjectionBlock.Create(mRenderer->GetDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		effect->BindUniformBuffer("UBO_viewProjection", &viewProjectionBlock);

		// Need clamp to edge when using transparent textures to not get artifacts at the top
		sampler = std::make_shared<Vk::Sampler>(mRenderer->GetDevice(), false);
		sampler->createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler->createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler->Create();

		Vk::Texture* texture = Vk::gTextureLoader().LoadTexture("data/textures/billboards/grass_2.png");
		Vk::Texture* texture2 = Vk::gTextureLoader().LoadTexture("data/textures/billboards/n_grass_diff_0_03.png");
		Vk::TextureArray textureArray;
		textureArray.AddTexture(texture->imageView, sampler.get());
		textureArray.AddTexture(texture2->imageView, sampler.get());

		effect->BindCombinedImage("textureSampler", &textureArray);
	}

	void GrassJob::Render(Vk::Renderer* renderer, const JobInput& jobInput)
	{
		//return;
		viewProjectionBlock.data.eyePos = glm::vec4(jobInput.sceneInfo.eyePos, 1.0f);
		viewProjectionBlock.data.view = jobInput.sceneInfo.viewMatrix;
		viewProjectionBlock.data.projection = jobInput.sceneInfo.projectionMatrix;
		viewProjectionBlock.data.grassViewDistance = jobInput.renderingSettings.grassViewDistance;
		viewProjectionBlock.UpdateMemory();

		/* Render instances */
		renderTarget->Begin();
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		commandBuffer->CmdBindPipeline(effect->GetPipeline());
		effect->BindDescriptorSets(commandBuffer);

		// Loop over all blocks and render their grass instance buffers
		auto blocks = jobInput.sceneInfo.terrain->GetBlocks();

		for(auto& iter : blocks)
		{
			if (iter.second->grassGenerated && iter.second->grassVisible)
			{
				commandBuffer->CmdBindVertexBuffer(0, 1, iter.second->instanceBuffer.get());
				commandBuffer->CmdDraw(4, iter.second->grassInstances.size(), 0, 0);
			}
		}

		renderTarget->End(renderer->GetQueue());
	}
}
