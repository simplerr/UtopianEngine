#include "ActorInspector.h"
#include "core/components/Actor.h"
#include "core/components/CLight.h"
#include "core/components/CTransform.h"
#include "core/components/CRenderable.h"
#include "editor/ComponentInspector.h"
#include "imgui/imgui.h"
#include "vulkan/UIOverlay.h"

namespace Utopian
{
	ActorInspector::ActorInspector()
	{
		mActor = nullptr;
	}

	ActorInspector::~ActorInspector()
	{
	}

	void ActorInspector::UpdateUi()
	{
		if (mActor != nullptr)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
			ImGui::SetNextWindowPos(ImVec2(1050, 10));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing);
			ImGui::PushItemWidth(300.0f);

			// General actor information
			if (ImGui::CollapsingHeader("Actor"), ImGuiTreeNodeFlags_DefaultOpen)
			{
				static int component = 1;
				ImGui::Combo("Component", &component, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");

				if (ImGui::Button("Add"))
				{ 
					uint32_t a = component;
				}
			}

			for (auto& inspector : mComponentInspectors)
			{
				inspector->UpdateUi();
			}

			ImGui::PopItemWidth();

			// ImGui functions end here
			ImGui::End();
			ImGui::PopStyleVar();
		}
	}

	void ActorInspector::SetActor(Utopian::Actor* actor)
	{
		mActor = actor;

		ClearInspectors();
		AddInspectors();
	}

	void ActorInspector::AddInspectors()
	{
		CTransform* transform = mActor->GetComponent<CTransform>();
		if (transform != nullptr) {
			TransformInspector* inspector = new TransformInspector(transform);
			mComponentInspectors.push_back(inspector);
		}

		CLight* light = mActor->GetComponent<CLight>();
		if (light != nullptr) {
			LightInspector* inspector = new LightInspector(light);
			mComponentInspectors.push_back(inspector);
		}

		CRenderable* renderable = mActor->GetComponent<CRenderable>();
		if (renderable != nullptr) {
			RenderableInspector* inspector = new RenderableInspector(renderable);
			mComponentInspectors.push_back(inspector);
		}
	}

	void ActorInspector::ClearInspectors()
	{
		for (auto inspector : mComponentInspectors)
		{
			delete inspector;
		}

		mComponentInspectors.clear();
	}
}
