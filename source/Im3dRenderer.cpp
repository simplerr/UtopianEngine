#include "Im3dRenderer.h"
#include "core/renderer/Renderer.h"
#include "core/renderer/RendererUtility.h"
#include "Camera.h"
#include "Input.h"
#include "im3d/im3d.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/EffectManager.h"
#include "vulkan/Effect.h"
#include "vulkan/VertexDescription.h"
#include "vulkan/handles/CommandBuffer.h"

namespace Utopian
{
	Im3dRenderer::Im3dRenderer(Vk::VulkanApp* vulkanApp, glm::vec2 viewportSize)
	{
		mViewportSize = viewportSize;
		mVulkanApp = vulkanApp;
	
		mCommandBuffer = new Vk::CommandBuffer(mVulkanApp->GetDevice(), VK_COMMAND_BUFFER_LEVEL_SECONDARY);
		vulkanApp->AddSecondaryCommandBuffer(mCommandBuffer);

		Vk::ShaderCreateInfo createInfo;
		createInfo.vertexShaderPath = "data/shaders/im3d/im3d.vert";

		mViewProjectionBlock.Create(mVulkanApp->GetDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mViewportBlock.Create(mVulkanApp->GetDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

		// Need to override vertex input description from shader since there is some special
		// treatment of U32 -> vec4 in Im3d
		mVertexDescription = std::make_shared<Vk::VertexDescription>();
		mVertexDescription->AddBinding(BINDING_0, sizeof(Im3d::VertexData), VK_VERTEX_INPUT_RATE_VERTEX);
		mVertexDescription->AddAttribute(BINDING_0, Vk::Vec4Attribute());
		mVertexDescription->AddAttribute(BINDING_0, Vk::U32Attribute());

		createInfo.fragmentShaderPath = "data/shaders/im3d/im3d_points.frag";
		mPointsEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mVulkanApp->GetDevice(), mVulkanApp->GetRenderPass(), createInfo);
		mPointsEffect->GetPipeline()->inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		mPointsEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_NONE;
		mPointsEffect->GetPipeline()->depthStencilState.depthTestEnable = VK_FALSE;
		mPointsEffect->GetPipeline()->depthStencilState.depthWriteEnable = VK_FALSE;
		mPointsEffect->GetPipeline()->OverrideVertexInput(mVertexDescription);
		gRendererUtility().SetAlphaBlending(mPointsEffect->GetPipeline());
		mPointsEffect->CreatePipeline();

		createInfo.fragmentShaderPath = "data/shaders/im3d/im3d_triangles.frag";
		mTrianglesEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mVulkanApp->GetDevice(), mVulkanApp->GetRenderPass(), createInfo);
		mTrianglesEffect->GetPipeline()->inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		mTrianglesEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_NONE;
		mTrianglesEffect->GetPipeline()->depthStencilState.depthTestEnable = VK_FALSE;
		mTrianglesEffect->GetPipeline()->depthStencilState.depthWriteEnable = VK_FALSE;
		mTrianglesEffect->GetPipeline()->OverrideVertexInput(mVertexDescription);
		gRendererUtility().SetAlphaBlending(mTrianglesEffect->GetPipeline());
		mTrianglesEffect->CreatePipeline();

		createInfo.geometryShaderPath = "data/shaders/im3d/im3d.geom";
		createInfo.fragmentShaderPath = "data/shaders/im3d/im3d_lines.frag";
		mLinesEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mVulkanApp->GetDevice(), mVulkanApp->GetRenderPass(), createInfo);
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

		mVertexCount = 0;
	}

	Im3dRenderer::~Im3dRenderer()
	{
		delete mCommandBuffer;
	}

	void Im3dRenderer::NewFrame()
	{
		Im3d::AppData& appData = Im3d::GetAppData();
		Camera* camera = gRenderer().GetMainCamera();

		//appData.m_deltaTime = 0.0f; // Todo
		appData.m_worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
		appData.m_viewOrigin = camera->GetPosition();
		appData.m_viewDirection = camera->GetDirection();
		appData.m_cursorRayOrigin = camera->GetPickingRay().origin;
		appData.m_cursorRayDirection = camera->GetPickingRay().direction;
		appData.m_projOrtho = false;
		appData.m_projScaleY = tanf(glm::radians(camera->GetFov()) * 0.5f) * 2.0f;
		appData.m_viewportSize = mViewportSize;
		appData.m_snapTranslation = 0.0f;
		appData.m_snapRotation = 0.0f;
		appData.m_snapScale = 0.0f;

		bool ctrlDown = gInput().KeyDown(VK_LCONTROL);
		appData.m_keyDown[Im3d::Key_L] = ctrlDown && gInput().KeyPressed('L');
		appData.m_keyDown[Im3d::Key_T] = ctrlDown && gInput().KeyPressed('T');
		appData.m_keyDown[Im3d::Key_R] = ctrlDown && gInput().KeyPressed('R');
		appData.m_keyDown[Im3d::Key_S] = ctrlDown && gInput().KeyPressed('S');

		Im3d::NewFrame();
	}

	void Im3dRenderer::EndFrame()
	{
		// Testing
		Im3d::SetSize(1.0f);
		glm::mat4 transform = glm::mat4();
		Im3d::Gizmo("TransformGizmo", Im3d::Mat4(transform));

		Im3d::SetSize(3.0f);
		Im3d::DrawAlignedBox(glm::vec3(0.0f), glm::vec3(500.0f));
		Im3d::DrawLine(glm::vec3(0.0f), glm::vec3(500.0f), 5.0f, Im3d::Color_Green);
		Im3d::DrawPoint(glm::vec3(0.0f, 500.0f, 0.0f), 20.0f, Im3d::Color_Red);
		Im3d::DrawCapsule(glm::vec3(0.0f), glm::vec3(0.0f, 1000.0f, 0.0f), 100.0f, 20);
		Im3d::DrawPrism(Im3d::Vec3(500.0f, 0.0f, 0.0f), Im3d::Vec3(500.0f, 500.0f, 0.0f), 100, 10);
		Im3d::DrawQuad(glm::vec3(-500.0f, 0.0f, 0.0f), Im3d::Vec3(0.0f, 1.0f, 0.0f), 300.0f);
		Im3d::DrawQuadFilled(glm::vec3(-1000.0f, 0.0f, 0.0f), Im3d::Vec3(0.0f, 1.0f, 0.0f), 300.0f);

		Im3d::BeginTriangles();
		Im3d::Vertex(-100.0f, 0.0f, -100.0f, Im3d::Color_Red);
		Im3d::Vertex(0.0f, 200.0f, -100.0f, Im3d::Color_Green);
		Im3d::Vertex(100.0f, 0.0f, -100.0f, Im3d::Color_Blue);
		Im3d::End();

		Im3d::EndFrame();

		UploadVertexData();
	}

	void Im3dRenderer::UploadVertexData()
	{
		Im3d::AppData& appData = Im3d::GetAppData();

		uint32_t totalNumVertices = GetTotalNumVertices();

		if ((mVertexBuffer.GetVkBuffer() == VK_NULL_HANDLE) || (mVertexCount < totalNumVertices))
		{
			VkDeviceSize vertexBufferSize = totalNumVertices * sizeof(Im3d::VertexData);

			mVertexBuffer.UnmapMemory();
			// Todo
			//gRenderer().QueueDestroy(Q)
			//mVertexBuffer.Destroy();
			mVertexBuffer.Create(mVulkanApp->GetDevice(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertexBufferSize);
			mVertexCount = totalNumVertices;
			mVertexBuffer.MapMemory(0, VK_WHOLE_SIZE, 0, (void**)&mMappedVertices);
			//updateCmdBuffers = true;
		}

		Im3d::VertexData* vertexDst = mMappedVertices;

		for (uint32_t i = 0; i < Im3d::GetDrawListCount(); i++)
		{
			const Im3d::DrawList& drawList = Im3d::GetDrawLists()[i];

			memcpy(vertexDst, drawList.m_vertexData, drawList.m_vertexCount * sizeof(Im3d::VertexData));
			vertexDst += drawList.m_vertexCount;
		}

		mVertexBuffer.Flush();
	}

	uint32_t Im3dRenderer::GetTotalNumVertices()
	{
		uint32_t numVertices = 0;
		for (uint32_t i = 0; i < Im3d::GetDrawListCount(); i++)
		{
			const Im3d::DrawList& drawList = Im3d::GetDrawLists()[i];
			numVertices += drawList.m_vertexCount;
		}

		return numVertices;
	}

	void Im3dRenderer::Render()
	{
		mViewProjectionBlock.data.view = gRenderer().GetMainCamera()->GetView();
		mViewProjectionBlock.data.projection = gRenderer().GetMainCamera()->GetProjection();
		mViewProjectionBlock.UpdateMemory();

		mViewportBlock.data.viewport.x = mViewportSize.x;
		mViewportBlock.data.viewport.y = mViewportSize.y;
		mViewportBlock.UpdateMemory();

		mCommandBuffer->Begin(mVulkanApp->GetRenderPass(), mVulkanApp->GetCurrentFrameBuffer());
		mCommandBuffer->CmdSetViewPort(mViewportSize.x, mViewportSize.y);
		mCommandBuffer->CmdSetScissor(mViewportSize.x, mViewportSize.y);
		mCommandBuffer->CmdBindVertexBuffer(0, 1, &mVertexBuffer);

		uint32_t vertexOffset = 0;
		for (uint32_t i = 0; i < Im3d::GetDrawListCount(); i++)
		{
			const Im3d::DrawList& drawList = Im3d::GetDrawLists()[i];

			if (drawList.m_primType == Im3d::DrawPrimitive_Lines)
			{
				mCommandBuffer->CmdBindDescriptorSets(mLinesEffect);
				mCommandBuffer->CmdBindPipeline(mLinesEffect->GetPipeline());
			}
			else if (drawList.m_primType == Im3d::DrawPrimitive_Points)
			{
				mCommandBuffer->CmdBindDescriptorSets(mPointsEffect);
				mCommandBuffer->CmdBindPipeline(mPointsEffect->GetPipeline());
			}
			else if (drawList.m_primType == Im3d::DrawPrimitive_Triangles)
			{
				mCommandBuffer->CmdBindDescriptorSets(mTrianglesEffect);
				mCommandBuffer->CmdBindPipeline(mTrianglesEffect->GetPipeline());
			}

			mCommandBuffer->CmdDraw(drawList.m_vertexCount, 1, vertexOffset, 0);

			vertexOffset += drawList.m_vertexCount;
		}

		mCommandBuffer->End();
	}
}