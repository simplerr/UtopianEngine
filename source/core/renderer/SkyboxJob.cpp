#include "core/renderer/SkyboxJob.h"
#include "core/renderer/DeferredJob.h"
#include "core/renderer/GBufferJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "vulkan/TextureLoader.h"

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
		mRenderTarget->AddReadWriteColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		mRenderTarget->AddReadWriteDepthAttachment(gbuffer.depthImage);
		mRenderTarget->SetClearColor(1, 1, 1, 1);
		mRenderTarget->Create();

		mSkybox = Vk::gTextureLoader().LoadCubemapTexture("data/textures/cubemap_space.ktx");

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/skybox/skybox.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/skybox/skybox.frag";
		mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), shaderCreateInfo);
		mEffect->GetPipeline()->depthStencilState.depthWriteEnable = VK_FALSE;
		mEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mEffect->CreatePipeline();

		mInputBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		mEffect->BindUniformBuffer("UBO_sharedVariables", gRenderer().GetSharedShaderVariables());
		mEffect->BindUniformBuffer("UBO_input", mInputBlock);
		mEffect->BindCombinedImage("samplerCubeMap", *mSkybox);

		mCubeModel = Vk::gModelLoader().LoadDebugBoxTriangles();
	}

	void SkyboxJob::Render(const JobInput& jobInput)
	{
		mInputBlock.data.world = glm::scale(glm::mat4(), glm::vec3(10000.0f));
		mInputBlock.UpdateMemory();

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
