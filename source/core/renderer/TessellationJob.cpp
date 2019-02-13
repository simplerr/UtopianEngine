#include "core/renderer/TessellationJob.h"
#include "core/renderer/SunShaftJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "vulkan/ShaderFactory.h"
#include "vulkan/ModelLoader.h"
#include "vulkan/ScreenQuadUi.h"
#include <random>

namespace Utopian
{
	TessellationJob::TessellationJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
		image = std::make_shared<Vk::ImageColor>(device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT);

		renderTarget = std::make_shared<Vk::RenderTarget>(device, width, height);
		renderTarget->AddColorAttachment(image);
		renderTarget->SetClearColor(1, 1, 1, 1);
		renderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/tessellation/tessellation.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/tessellation/tessellation.frag";
		shaderCreateInfo.tescShaderPath = "data/shaders/tessellation/tessellation.tesc";
		shaderCreateInfo.teseShaderPath = "data/shaders/tessellation/tessellation.tese";
		mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(device, renderTarget->GetRenderPass(), shaderCreateInfo);

		mEffect->GetPipeline()->rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;

		mEffect->CreatePipeline();

		viewProjectionBlock.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mEffect->BindUniformBuffer("UBO_viewProjection", &viewProjectionBlock);

		settingsBlock.Create(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		settingsBlock.data.color = glm::vec3(0, 0, 1);
		mEffect->BindUniformBuffer("UBO_settings", &settingsBlock);

		mQuadModel = Vk::gModelLoader().LoadGrid(512.0f, 2);

		const uint32_t size = 640;
		gScreenQuadUi().AddQuad(size + 20, height - (size + 310), size, size, image.get(), renderTarget->GetSampler());
	}

	TessellationJob::~TessellationJob()
	{
	}

	void TessellationJob::Init(const std::vector<BaseJob*>& renderers)
	{
		SunShaftJob* sunShaftJob = static_cast<SunShaftJob*>(renderers[JobGraph::SUN_SHAFT_INDEX]);

		SetWaitSemaphore(sunShaftJob->GetCompletedSemahore());
	}

	void TessellationJob::Render(const JobInput& jobInput)
	{
		//effect->SetCameraData(jobInput.sceneInfo.viewMatrix, jobInput.sceneInfo.projectionMatrix, glm::vec4(jobInput.sceneInfo.eyePos, 1.0f));
		//effect->SetSettings(jobInput.renderingSettings.ssaoRadius, jobInput.renderingSettings.ssaoBias);
		viewProjectionBlock.data.view = jobInput.sceneInfo.viewMatrix;
		viewProjectionBlock.data.projection = jobInput.sceneInfo.projectionMatrix;
		viewProjectionBlock.UpdateMemory();

		renderTarget->Begin("Tessellation pass", glm::vec4(0.0, 1.0, 0.0, 1.0));
		Vk::CommandBuffer* commandBuffer = renderTarget->GetCommandBuffer();

		if (IsEnabled())
		{
			// Push the world matrix constant
			glm::mat4 world = glm::mat4();
			Vk::PushConstantBlock pushConsts(world);

			commandBuffer->CmdPushConstants(mEffect->GetPipelineInterface(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConsts), &pushConsts);
			commandBuffer->CmdBindPipeline(mEffect->GetPipeline());
			commandBuffer->CmdBindDescriptorSets(mEffect);

			Vk::Mesh* mesh = mQuadModel->mMeshes[0];
			commandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
			commandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
			commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
		}

		renderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
	}
}
