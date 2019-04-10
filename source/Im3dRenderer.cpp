#include "Im3dRenderer.h"
#include "core/renderer/Renderer.h"
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
		//createInfo.geometryShaderPath = "data/shaders/im3d/im3d.geom";
		createInfo.fragmentShaderPath = "data/shaders/im3d/im3d.frag";
		mEffect = Vk::gEffectManager().AddEffect<Vk::Effect>(mVulkanApp->GetDevice(), mVulkanApp->GetRenderPass(), createInfo);
		mEffect->GetPipeline()->inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		mEffect->GetPipeline()->rasterizationState.cullMode = VK_CULL_MODE_NONE;
		mEffect->GetPipeline()->depthStencilState.depthTestEnable = VK_FALSE;
		mEffect->GetPipeline()->depthStencilState.depthWriteEnable = VK_FALSE;

		// Need to override vertex input description from shader since there is some special
		// treatment of U32 -> vec4 in Im3d
		mVertexDescription = std::make_shared<Vk::VertexDescription>();
		mVertexDescription->AddBinding(BINDING_0, sizeof(Im3d::VertexData), VK_VERTEX_INPUT_RATE_VERTEX);
		mVertexDescription->AddAttribute(BINDING_0, Vk::Vec4Attribute());
		mVertexDescription->AddAttribute(BINDING_0, Vk::U32Attribute());
		mEffect->GetPipeline()->OverrideVertexInput(mVertexDescription);

		mEffect->CreatePipeline();

		mViewProjectionBlock.Create(mVulkanApp->GetDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		mEffect->BindUniformBuffer("UBO_viewProjection", &mViewProjectionBlock);
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
		// Needed for constant screen space size gizmos
		//appData.m_projScaleY = 
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
		Im3d::DrawAlignedBox(glm::vec3(0.0f), glm::vec3(500.0f));

		Im3d::EndFrame();

		UploadVertexData();
	}

	void Im3dRenderer::UploadVertexData()
	{
		Im3d::AppData& appData = Im3d::GetAppData();

		for (uint32_t i = 0; i < Im3d::GetDrawListCount(); i++)
		{
			const Im3d::DrawList& drawList = Im3d::GetDrawLists()[i];

			VkDeviceSize vertexBufferSize =  drawList.m_vertexCount * sizeof(Im3d::VertexData);

			// Update buffers only if vertex or index count has been changed compared to current buffer size
			if ((mVertexBuffer.GetVkBuffer() == VK_NULL_HANDLE) || (mVertexCount != drawList.m_vertexCount))
			{
				mVertexBuffer.UnmapMemory();
				// Todo
				//gRenderer().QueueDestroy(Q)
				//mVertexBuffer.Destroy();
				mVertexBuffer.Create(mVulkanApp->GetDevice(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertexBufferSize);
				mVertexCount = drawList.m_vertexCount;
				mVertexBuffer.MapMemory(0, VK_WHOLE_SIZE, 0, (void**)&mMappedVertices);
				//updateCmdBuffers = true;
			}

			memcpy(mMappedVertices, drawList.m_vertexData, vertexBufferSize);

			mVertexBuffer.Flush();

			volatile int a = 1;
		}

	}

	void Im3dRenderer::Render()
	{
		mViewProjectionBlock.data.view = gRenderer().GetMainCamera()->GetView();
		mViewProjectionBlock.data.projection = gRenderer().GetMainCamera()->GetProjection();
		mViewProjectionBlock.UpdateMemory();

		mCommandBuffer->Begin(mVulkanApp->GetRenderPass(), mVulkanApp->GetCurrentFrameBuffer());
		mCommandBuffer->CmdSetViewPort(mViewportSize.x, mViewportSize.y);
		mCommandBuffer->CmdSetScissor(mViewportSize.x, mViewportSize.y);
		mCommandBuffer->CmdBindPipeline(mEffect->GetPipeline());

		mCommandBuffer->CmdBindVertexBuffer(0, 1, &mVertexBuffer);
		mCommandBuffer->CmdBindDescriptorSets(mEffect);
		mCommandBuffer->CmdDraw(mVertexCount, 1, 0, 0);


		mCommandBuffer->End();
		//Im3d::AppData& appData = Im3d::GetAppData();

		/*for (uint32_t i = 0; i < Im3d::GetDrawListCount(); i++)
		{
			const Im3d::DrawList& drawList = Im3d::GetDrawLists()[i];

			volatile int a = 1;
		}*/
	}
}