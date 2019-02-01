#include "core/renderer/GrassJob.h"
#include "core/renderer/DeferredJob.h"
#include "core/renderer/GBufferJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "core/terrain/PerlinTerrain.h"

namespace Utopian
{
	GrassJob::GrassJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		//effect->BindCombinedImage("textureSampler", todo)
	}

	GrassJob::~GrassJob()
	{
	}

	void GrassJob::Init(const std::vector<BaseJob*>& jobs)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);
		GBufferJob* gbufferJob = static_cast<GBufferJob*>(jobs[JobGraph::GBUFFER_INDEX]);

		renderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
		renderTarget->AddColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ATTACHMENT_LOAD_OP_LOAD);
		renderTarget->AddDepthAttachment(gbufferJob->depthImage);
		renderTarget->GetRenderPass()->attachments[Vk::RenderPassAttachment::DEPTH_ATTACHMENT].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		effect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice,
			renderTarget->GetRenderPass(),
			"data/shaders/grass/grass.vert",
			"data/shaders/grass/grass.frag");

		SharedPtr<Vk::VertexDescription> vertexDescription = std::make_shared<Vk::VertexDescription>();

		vertexDescription->AddBinding(BINDING_0, sizeof(GrassInstance), VK_VERTEX_INPUT_RATE_INSTANCE);
		vertexDescription->AddAttribute(BINDING_0, Vk::Vec4Attribute());	// Location 0 : InstancePos
		vertexDescription->AddAttribute(BINDING_0, Vk::Vec3Attribute());	// Location 1 : Color
		vertexDescription->AddAttribute(BINDING_0, Vk::S32Attribute());		// Location 2 : InTexId

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

		viewProjectionBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		effect->BindUniformBuffer("UBO_viewProjection", &viewProjectionBlock);

		// Need clamp to edge when using transparent textures to not get artifacts at the top
		sampler = std::make_shared<Vk::Sampler>(mDevice, false);
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

	void GrassJob::Render(const JobInput& jobInput)
	{
		viewProjectionBlock.data.eyePos = glm::vec4(jobInput.sceneInfo.eyePos, 1.0f);
		viewProjectionBlock.data.view = jobInput.sceneInfo.viewMatrix;
		viewProjectionBlock.data.projection = jobInput.sceneInfo.projectionMatrix;
		viewProjectionBlock.data.grassViewDistance = jobInput.renderingSettings.grassViewDistance;
		viewProjectionBlock.UpdateMemory();

		/* Render instances */
		renderTarget->Begin();
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		commandBuffer->CmdBindPipeline(effect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(effect);

		// Loop over all blocks and render their grass instance buffers
		auto blocks = jobInput.sceneInfo.terrain->GetBlocks();

		for (auto& iter : blocks)
		{
			if (iter.second->grassGenerated && iter.second->grassVisible)
			{
				commandBuffer->CmdBindVertexBuffer(0, 1, iter.second->instanceBuffer.get());
				commandBuffer->CmdDraw(4, iter.second->grassInstances.size(), 0, 0);
			}
		}

		renderTarget->End();
	}
}
