#include "core/renderer/GeometryThicknessJob.h"
#include "core/renderer/SSAOJob.h"
#include "core/renderer/CommonJobIncludes.h"

namespace Utopian
{
	GeometryThicknessJob::GeometryThicknessJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
	
	}

	GeometryThicknessJob::~GeometryThicknessJob()
	{
	}

	void GeometryThicknessJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		geometryThicknessImage = std::make_shared<Vk::ImageColor>(mDevice, mWidth, mHeight, VK_FORMAT_R32_SFLOAT);

		mRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
		mRenderTarget->AddWriteOnlyColorAttachment(geometryThicknessImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		mRenderTarget->SetClearColor(DEFAULT_THICKNESS, 0.0f, 0.0f, 0.0f);
		mRenderTarget->Create();

		Vk::ShaderCreateInfo shaderCreateInfo;
		shaderCreateInfo.vertexShaderPath = "data/shaders/geometry_thickness/geometry_thickness.vert";
		shaderCreateInfo.fragmentShaderPath = "data/shaders/geometry_thickness/geometry_thickness.frag";

		mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), shaderCreateInfo);
		mEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		mEffect->CreatePipeline();

		mViewProjectionBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mEffect->BindUniformBuffer("UBO_viewProjection", &mViewProjectionBlock);
		mEffect->BindCombinedImage("depthSampler", gbuffer.depthImage.get(), mRenderTarget->GetSampler());

		// const uint32_t size = 640;
		// gScreenQuadUi().AddQuad(10, mHeight - (size + 10), size, size, geometryThicknessImage.get(), mRenderTarget->GetSampler());
	}

	void GeometryThicknessJob::Render(const JobInput& jobInput)
	{
		mViewProjectionBlock.data.view = jobInput.sceneInfo.viewMatrix;
		mViewProjectionBlock.data.projection = jobInput.sceneInfo.projectionMatrix;
		mViewProjectionBlock.UpdateMemory();

		mRenderTarget->Begin("Geometry thickness pass", glm::vec4(0.5, 1.0, 1.0, 1.0));
		Vk::CommandBuffer* commandBuffer = mRenderTarget->GetCommandBuffer();

		// Todo: Should this be moved to the effect instead?
		commandBuffer->CmdBindPipeline(mEffect->GetPipeline());

		// Todo: Instanced objects

		/* Render all renderables */
		for (auto& renderable : jobInput.sceneInfo.renderables)
		{
			if (!renderable->IsVisible() || ((renderable->GetRenderFlags() & RENDER_FLAG_DEFERRED) != RENDER_FLAG_DEFERRED))
				continue;

			Vk::StaticModel * model = renderable->GetModel();

			for (Vk::Mesh* mesh : model->mMeshes)
			{
				Vk::PushConstantBlock pushConsts(renderable->GetTransform().GetWorldMatrix(), renderable->GetColor());
				commandBuffer->CmdPushConstants(mEffect->GetPipelineInterface(), VK_SHADER_STAGE_ALL, sizeof(pushConsts), &pushConsts);

				VkDescriptorSet textureDescriptorSet = mesh->GetTextureDescriptorSet();
				VkDescriptorSet descriptorSets[2] = { mEffect->GetDescriptorSet(0).GetVkHandle(), textureDescriptorSet };
				commandBuffer->CmdBindDescriptorSet(mEffect->GetPipelineInterface(), 2, descriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);

				commandBuffer->CmdBindVertexBuffer(0, 1, mesh->GetVertxBuffer());
				commandBuffer->CmdBindIndexBuffer(mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
				commandBuffer->CmdDrawIndexed(mesh->GetNumIndices(), 1, 0, 0, 0);
			}
		}

		mRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
	}
}
