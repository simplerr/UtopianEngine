#include "core/renderer/SkydomeJob.h"
#include "core/renderer/DeferredJob.h"
#include "core/renderer/GBufferJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "core/renderer/Light.h"

namespace Utopian
{
	SkydomeJob::SkydomeJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		parameterBlock.data.inclination = 90.0f;
		parameterBlock.data.azimuth = 0.0f;
		sunAzimuth = 0.0f;

		sunImage = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R8G8B8A8_UNORM);
	}

	SkydomeJob::~SkydomeJob()
	{
	}

	void SkydomeJob::Init(const std::vector<BaseJob*>& renderers)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(renderers[JobGraph::DEFERRED_INDEX]);
		GBufferJob* gbufferJob = static_cast<GBufferJob*>(renderers[JobGraph::GBUFFER_INDEX]);

		renderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
		renderTarget->AddColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_LOAD);
		renderTarget->AddColorAttachment(sunImage);
		renderTarget->AddDepthAttachment(gbufferJob->depthImage, VK_ATTACHMENT_LOAD_OP_LOAD);
		renderTarget->SetClearColor(0, 0, 0);
		renderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/skydome/skydome.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/skydome/skydome.frag";
		effect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, renderTarget->GetRenderPass(), shaderCreateInfo);

		effect->GetPipeline()->depthStencilState.depthWriteEnable = VK_FALSE;
		effect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		//effect->GetPipeline()->rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		effect->CreatePipeline();

		viewProjectionBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		parameterBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		effect->BindUniformBuffer("UBO_viewProjection", &viewProjectionBlock);
		effect->BindUniformBuffer("UBO_parameters", &parameterBlock);

		mSkydomeModel = Vk::gModelLoader().LoadModel("data/models/sphere.obj");

		SetWaitSemaphore(deferredJob->GetCompletedSemahore());
	}

	void SkydomeJob::Render(const JobInput& jobInput)
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
		glm::mat4 world = glm::rotate(glm::mat4(), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		viewProjectionBlock.data.view = glm::mat4(glm::mat3(jobInput.sceneInfo.viewMatrix));
		viewProjectionBlock.data.projection = jobInput.sceneInfo.projectionMatrix;
		viewProjectionBlock.data.world = glm::scale(world, glm::vec3(1000.0f));
		viewProjectionBlock.UpdateMemory();

		parameterBlock.data.sphereRadius = mSkydomeModel->GetBoundingBox().GetHeight() / 2.0f;
		parameterBlock.data.inclination = sunInclination;
		parameterBlock.data.azimuth = sunAzimuth;
		parameterBlock.data.time = Timer::Instance().GetTime();
		parameterBlock.data.sunSpeed = jobInput.renderingSettings.sunSpeed;
		parameterBlock.data.onlySun = false;
		parameterBlock.UpdateMemory();

		renderTarget->Begin("Skydome pass", glm::vec4(0.3, 0.8, 1.0, 1.0));
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		// Todo: Should this be moved to the effect instead?
		commandBuffer->CmdBindPipeline(effect->GetPipeline());
		commandBuffer->CmdBindDescriptorSets(effect);

		commandBuffer->CmdBindVertexBuffer(0, 1, mSkydomeModel->mMeshes[0]->GetVertxBuffer());
		commandBuffer->CmdBindIndexBuffer(mSkydomeModel->mMeshes[0]->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
		commandBuffer->CmdDrawIndexed(mSkydomeModel->GetNumIndices(), 1, 0, 0, 0);

		renderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
	}
}
