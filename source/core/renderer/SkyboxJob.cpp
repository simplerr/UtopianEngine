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

	void SkyboxJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);

		mRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
		mRenderTarget->AddReadWriteColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		mRenderTarget->AddReadWriteDepthAttachment(gbuffer.depthImage);
		mRenderTarget->SetClearColor(1, 1, 1, 1);
		mRenderTarget->Create();

		mSkybox = std::make_shared<Vk::CubeMapTexture>();
		mSkybox->LoadFromFile("data/textures/cubemap_space.ktx", VK_FORMAT_R8G8B8A8_UNORM, mDevice, mDevice->GetQueue());

		mEffect = Vk::gEffectManager().AddEffect<Vk::SkyboxEffect>(mDevice, mRenderTarget->GetRenderPass());
		mEffect->BindCombinedImage("samplerCubeMap", mSkybox->image, mRenderTarget->GetSampler()); // skybox->sampler);

		mCubeModel = Vk::gModelLoader().LoadDebugBoxTriangles();
	}

	void SkyboxJob::Render(const JobInput& jobInput)
	{
		mEffect->SetCameraData(jobInput.sceneInfo.viewMatrix, jobInput.sceneInfo.projectionMatrix);

		mRenderTarget->Begin();
		Vk::CommandBuffer* commandBuffer = mRenderTarget->GetCommandBuffer();

		commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(mEffect);

		commandBuffer->CmdBindVertexBuffer(0, 1, mCubeModel->mMeshes[0]->GetVertxBuffer());
		commandBuffer->CmdBindIndexBuffer(mCubeModel->mMeshes[0]->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
		commandBuffer->CmdDrawIndexed(mCubeModel->GetNumIndices(), 1, 0, 0, 0);

		mRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
	}
}
