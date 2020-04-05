#include "core/renderer/Im3dJob.h"
#include "core/renderer/DeferredJob.h"
#include "core/renderer/CommonJobIncludes.h"
#include "im3d/im3d.h"

namespace Utopian
{
	Im3dJob::Im3dJob(Vk::Device* device, uint32_t width, uint32_t height)
		: BaseJob(device, width, height)
	{
	}

	Im3dJob::~Im3dJob()
	{
	}

	void Im3dJob::Init(const std::vector<BaseJob*>& jobs, const GBuffer& gbuffer)
	{
		DeferredJob* deferredJob = static_cast<DeferredJob*>(jobs[JobGraph::DEFERRED_INDEX]);

		mRenderTarget = std::make_shared<Vk::RenderTarget>(mDevice, mWidth, mHeight);
		mRenderTarget->AddReadWriteColorAttachment(deferredJob->renderTarget->GetColorImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		mRenderTarget->AddReadWriteDepthAttachment(gbuffer.depthImage);//, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		mRenderTarget->SetClearColor(1, 1, 1, 1);
		mRenderTarget->Create();

		Vk::ShaderCreateInfo createInfo;
		createInfo.vertexShaderPath = "data/shaders/im3d/im3d.vert";

		mViewProjectionBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mViewportBlock.Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		// Need to override vertex input description from shader since there is some special
		// treatment of U32 -> vec4 in Im3d
		mVertexDescription = std::make_shared<Vk::VertexDescription>();
		mVertexDescription->AddBinding(BINDING_0, sizeof(Im3d::VertexData), VK_VERTEX_INPUT_RATE_VERTEX);
		mVertexDescription->AddAttribute(BINDING_0, Vk::Vec4Attribute());
		mVertexDescription->AddAttribute(BINDING_0, Vk::U32Attribute());

		createInfo.fragmentShaderPath = "data/shaders/im3d/im3d_points.frag";
		mPointsEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), createInfo);
		mPointsEffect->GetPipeline()->inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		mPointsEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_NONE;
		mPointsEffect->GetPipeline()->depthStencilState.depthTestEnable = VK_FALSE;
		mPointsEffect->GetPipeline()->depthStencilState.depthWriteEnable = VK_FALSE;
		mPointsEffect->GetPipeline()->OverrideVertexInput(mVertexDescription);
		gRendererUtility().SetAlphaBlending(mPointsEffect->GetPipeline());
		mPointsEffect->CreatePipeline();

		createInfo.fragmentShaderPath = "data/shaders/im3d/im3d_triangles.frag";
		mTrianglesEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), createInfo);
		mTrianglesEffect->GetPipeline()->inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		mTrianglesEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_NONE;
		mTrianglesEffect->GetPipeline()->depthStencilState.depthTestEnable = VK_FALSE;
		mTrianglesEffect->GetPipeline()->depthStencilState.depthWriteEnable = VK_FALSE;
		mTrianglesEffect->GetPipeline()->OverrideVertexInput(mVertexDescription);
		gRendererUtility().SetAlphaBlending(mTrianglesEffect->GetPipeline());
		mTrianglesEffect->CreatePipeline();

		createInfo.geometryShaderPath = "data/shaders/im3d/im3d.geom";
		createInfo.fragmentShaderPath = "data/shaders/im3d/im3d_lines.frag";
		mLinesEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mDevice, mRenderTarget->GetRenderPass(), createInfo);
		mLinesEffect->GetPipeline()->inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		mLinesEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_NONE;
		mLinesEffect->GetPipeline()->depthStencilState.depthTestEnable = VK_FALSE;
		mLinesEffect->GetPipeline()->depthStencilState.depthWriteEnable = VK_FALSE;
		mLinesEffect->GetPipeline()->OverrideVertexInput(mVertexDescription);
		gRendererUtility().SetAlphaBlending(mLinesEffect->GetPipeline());
		mLinesEffect->CreatePipeline();

		mLinesEffect->BindUniformBuffer("UBO_viewProjection", &mViewProjectionBlock);
		mPointsEffect->BindUniformBuffer("UBO_viewProjection", &mViewProjectionBlock);
		mTrianglesEffect->BindUniformBuffer("UBO_viewProjection", &mViewProjectionBlock);

		mLinesEffect->BindUniformBuffer("UBO_viewport", &mViewportBlock);
	}

	void Im3dJob::Render(const JobInput& jobInput)
	{
		mViewProjectionBlock.data.view = jobInput.sceneInfo.viewMatrix;
		mViewProjectionBlock.data.projection = jobInput.sceneInfo.projectionMatrix;
		mViewProjectionBlock.UpdateMemory();

		mViewportBlock.data.viewport.x = mWidth;
		mViewportBlock.data.viewport.y = mHeight;
		mViewportBlock.UpdateMemory();

		mRenderTarget->Begin("Im3d pass", glm::vec4(0.5, 1.0, 0.0, 1.0));

		Vk::CommandBuffer* commandBuffer = mRenderTarget->GetCommandBuffer();

		commandBuffer->CmdBindVertexBuffer(0, 1, jobInput.sceneInfo.im3dVertices.get());

		uint32_t vertexOffset = 0;
		for (uint32_t i = 0; i < Im3d::GetDrawListCount(); i++)
		{
			const Im3d::DrawList& drawList = Im3d::GetDrawLists()[i];

			if (drawList.m_primType == Im3d::DrawPrimitive_Lines)
			{
				commandBuffer->CmdBindDescriptorSets(mLinesEffect);
				commandBuffer->CmdBindPipeline(mLinesEffect->GetPipeline());
			}
			else if (drawList.m_primType == Im3d::DrawPrimitive_Points)
			{
				commandBuffer->CmdBindDescriptorSets(mPointsEffect);
				commandBuffer->CmdBindPipeline(mPointsEffect->GetPipeline());
			}
			else if (drawList.m_primType == Im3d::DrawPrimitive_Triangles)
			{
				commandBuffer->CmdBindDescriptorSets(mTrianglesEffect);
				commandBuffer->CmdBindPipeline(mTrianglesEffect->GetPipeline());
			}

			commandBuffer->CmdDraw(drawList.m_vertexCount, 1, vertexOffset, 0);

			vertexOffset += drawList.m_vertexCount;
		}

		mRenderTarget->End(GetWaitSemahore(), GetCompletedSemahore());
	}
}
