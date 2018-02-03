#include "Editor.h"
#include "Input.h"
#include "vulkan/Renderer.h"
#include "Camera.h"
#include "scene/World.h"
#include "scene/SceneEntity.h"
#include "scene/CTransform.h"

namespace Scene
{
	Editor::Editor(Vulkan::Renderer* renderer, World* world)
		: mRenderer(renderer), mWorld(world)
	{

	}

	Editor::~Editor()
	{
	}

	void Editor::Update()
	{
		// Was an Entity selected?
		if (gInput().KeyPressed(VK_LBUTTON))
		{
			Vulkan::Ray ray = mRenderer->GetCamera()->GetPickingRay();

			SceneEntity* selectedEntity = mWorld->RayIntersection(ray);
			if (selectedEntity != nullptr)
			{
				mSelectedEntity = selectedEntity;

				auto transform = selectedEntity->GetComponent<CTransform>();
				transform->SetScale(transform->GetScale() * 1.3f);
			}
		}
	}

	void Editor::UpdateUi()
	{
	}

	void Editor::Draw()
	{
	}
}
