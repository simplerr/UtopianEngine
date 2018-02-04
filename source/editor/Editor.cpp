#include "Editor.h"
#include "Input.h"
#include "vulkan/Renderer.h"
#include "Camera.h"
#include "scene/World.h"
#include "scene/Actor.h"
#include "scene/CTransform.h"
#include "scene/CRenderable.h"
#include "vulkan/UIOverlay.h"
#include "editor/ActorInspector.h"

namespace Scene
{
	Editor::Editor(Vulkan::Renderer* renderer, World* world)
		: mRenderer(renderer), mWorld(world)
	{
		mSelectedActor = nullptr;

		mActorInspector = new ActorInspector();
	}

	Editor::~Editor()
	{
		delete mActorInspector;
	}

	void Editor::Update()
	{
		UpdateUi();

		// Was an Entity selected?
		if (gInput().KeyPressed(VK_LBUTTON))
		{
			Vulkan::Ray ray = mRenderer->GetCamera()->GetPickingRay();

			Actor* selectedActor = mWorld->RayIntersection(ray);
			if (selectedActor != nullptr && selectedActor != mSelectedActor)
			{
				OnActorSelected(selectedActor);
			}
		}
	}

	void Editor::UpdateUi()
	{
		mActorInspector->UpdateUi();

		if (IsActorSelected())
		{
		}
	}

	void Editor::Draw()
	{
	}

	bool Editor::IsActorSelected()
	{
		return mSelectedActor != nullptr;
	}

	void Editor::OnActorSelected(Actor* actor)
	{
		if (IsActorSelected())
		{
			auto renderable = mSelectedActor->GetComponent<CRenderable>();
			renderable->DisableBoundingBox();
		}

		mSelectedActor = actor;

		// Enable bounding box rendering
		auto renderable = mSelectedActor->GetComponent<CRenderable>();
		renderable->EnableBoundingBox();

		// Create inspector UI
		mActorInspector->SetActor(mSelectedActor);
	}
}
