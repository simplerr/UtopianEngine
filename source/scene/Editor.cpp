#include "Editor.h"
#include "Input.h"
#include "vulkan/Renderer.h"
#include "Camera.h"
#include "scene/World.h"
#include "scene/Actor.h"
#include "scene/CTransform.h"
#include "scene/CRenderable.h"
#include "vulkan/UIOverlay.h"

namespace Scene
{
	Editor::Editor(Vulkan::Renderer* renderer, World* world)
		: mRenderer(renderer), mWorld(world)
	{
		mSelectedEntity = nullptr;
	}

	Editor::~Editor()
	{
	}

	void Editor::Update()
	{
		UpdateUi();

		// Was an Entity selected?
		if (gInput().KeyPressed(VK_LBUTTON))
		{
			Vulkan::Ray ray = mRenderer->GetCamera()->GetPickingRay();

			Actor* selectedEntity = mWorld->RayIntersection(ray);
			if (selectedEntity != nullptr)
			{
				if (IsEntitySelected())
				{
					auto renderable = mSelectedEntity->GetComponent<CRenderable>();
					renderable->DisableBoundingBox();
				}

				mSelectedEntity = selectedEntity;

				auto transform = selectedEntity->GetComponent<CTransform>();
				transform->SetScale(transform->GetScale() * 1.3f);

				auto renderable = selectedEntity->GetComponent<CRenderable>();
				renderable->EnableBoundingBox();
			}
		}
	}

	void Editor::UpdateUi()
	{
		if (IsEntitySelected())
		{
			Vulkan::UIOverlay::TextV("Entity selected = [%s]", mSelectedEntity->GetName().c_str());
		}
	}

	void Editor::Draw()
	{
	}

	bool Editor::IsEntitySelected()
	{
		return mSelectedEntity != nullptr;
	}
}
