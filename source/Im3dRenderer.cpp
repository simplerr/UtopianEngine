#include "Im3dRenderer.h"
#include "core/renderer/Renderer.h"
#include "Camera.h"
#include "Input.h"
#include "im3d/im3d.h"

namespace Utopian
{
	Im3dRenderer::Im3dRenderer(glm::vec2 viewportSize)
	{
		mViewportSize = viewportSize;
	}

	Im3dRenderer::~Im3dRenderer()
	{
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
		Im3d::DrawPoint(glm::vec3(0.0f), 50.0f, Im3d::Color_Black);
		Im3d::DrawAlignedBox(glm::vec3(0.0f), glm::vec3(500.0f));

		Im3d::EndFrame();

		Im3d::AppData& appData = Im3d::GetAppData();

		for (uint32_t i = 0; i < Im3d::GetDrawListCount(); i++)
		{
			const Im3d::DrawList& drawList = Im3d::GetDrawLists()[i];

			volatile int a = 1;
		}
	}
}