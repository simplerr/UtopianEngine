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

		mVertexBuffer = std::make_shared<Vk::Buffer>();
		mVertexCount = 0;
	}

	Im3dRenderer::~Im3dRenderer()
	{

	}

	SharedPtr<Vk::Buffer> Im3dRenderer::GetVertexBuffer()
	{
		return mVertexBuffer;
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
		Im3d::PushLayerId(5);
		Im3d::SetSize(1.0f);
		glm::mat4 transform = glm::mat4();
		Im3d::Gizmo("TransformGizmo", Im3d::Mat4(transform));

		Im3d::SetSize(3.0f);
		Im3d::DrawAlignedBox(glm::vec3(0.0f), glm::vec3(500.0f));
		Im3d::PopLayerId();
		Im3d::PushLayerId(10);
		Im3d::DrawLine(glm::vec3(0.0f), glm::vec3(500.0f), 5.0f, Im3d::Color_Green);
		Im3d::DrawPoint(glm::vec3(0.0f, 500.0f, 0.0f), 20.0f, Im3d::Color_Red);
		Im3d::DrawCapsule(glm::vec3(0.0f), glm::vec3(0.0f, 1000.0f, 0.0f), 100.0f, 20);
		Im3d::DrawPrism(Im3d::Vec3(500.0f, 0.0f, 0.0f), Im3d::Vec3(500.0f, 500.0f, 0.0f), 100, 10);
		Im3d::PopLayerId();

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

		if ((mVertexBuffer->GetVkBuffer() == VK_NULL_HANDLE) || (mVertexCount < totalNumVertices))
		{
			VkDeviceSize vertexBufferSize = totalNumVertices * sizeof(Im3d::VertexData);

			mVertexBuffer->UnmapMemory();
			// Todo
			//gRenderer().QueueDestroy(Q)
			//mVertexBuffer.Destroy();
			mVertexBuffer->Create(mVulkanApp->GetDevice(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertexBufferSize);
			mVertexCount = totalNumVertices;
			mVertexBuffer->MapMemory(0, VK_WHOLE_SIZE, 0, (void**)&mMappedVertices);
			//updateCmdBuffers = true;
		}

		Im3d::VertexData* vertexDst = mMappedVertices;

		for (uint32_t i = 0; i < Im3d::GetDrawListCount(); i++)
		{
			const Im3d::DrawList& drawList = Im3d::GetDrawLists()[i];

			memcpy(vertexDst, drawList.m_vertexData, drawList.m_vertexCount * sizeof(Im3d::VertexData));
			vertexDst += drawList.m_vertexCount;
		}

		mVertexBuffer->Flush();
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
}