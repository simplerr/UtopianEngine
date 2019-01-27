#include "core/renderer/SkyboxJob.h"
#include "core/renderer/DeferredJob.h"
#include "core/renderer/GBufferJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "vulkan/CubeMapTexture.h"

namespace Utopian
{
	SkyboxJob::SkyboxJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
	}

	SkyboxJob::~SkyboxJob()
	{
	}

	void SkyboxJob::Init(const std::vector<BaseJob*>& renderers)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(renderers[RenderingManager::DEFERRED_INDEX]);
		GBufferJob* gbufferJob = static_cast<GBufferJob*>(renderers[RenderingManager::GBUFFER_INDEX]);

		renderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
		renderTarget->AddColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ATTACHMENT_LOAD_OP_LOAD);
		renderTarget->AddDepthAttachment(gbufferJob->depthImage);
		// Todo: Investigate why this does not work
		renderTarget->GetRenderPass()->attachments[Vk::RenderPassAttachment::DEPTH_ATTACHMENT].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		skybox = std::make_shared<Vk::CubeMapTexture>();
		skybox->LoadFromFile("data/textures/cubemap_space.ktx", VK_FORMAT_R8G8B8A8_UNORM, mDevice, mDevice->GetQueue());

		effect = Vk::gEffectManager().AddEffect<Vk::SkyboxEffect>(mDevice, renderTarget->GetRenderPass());
		effect->BindCombinedImage("samplerCubeMap", skybox->image, renderTarget->GetSampler()); // skybox->sampler);

		mCubeModel = Vk::gModelLoader().LoadDebugBox();
	}

	void SkyboxJob::Render(const JobInput& jobInput)
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

		renderTarget->End();
	}
}
