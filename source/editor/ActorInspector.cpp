#include "ActorInspector.h"
#include "core/components/Actor.h"
#include "core/components/CLight.h"
#include "core/components/CTransform.h"
#include "core/components/CRenderable.h"
#include "core/components/CRigidBody.h"
#include "editor/ComponentInspector.h"
#include "imgui/imgui.h"
#include "ImGuiRenderer.h"

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
			ImGuiRenderer::BeginWindow("Inspector", glm::vec2(1500, 10), 300.0f);

			// General actor information
			std::string name = mActor->GetName();
			if (ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				// static int component = 1;
				// ImGui::Combo("Component", &component, "Light\0Renderable\0\0");

				// if (ImGui::Button("Add"))
				// { 
				// 	uint32_t a = component;
				// }
			}

			for (auto& inspector : mComponentInspectors)
			{
				inspector->UpdateUi();
			}

			ImGuiRenderer::EndWindow();
		}
	}

	void ActorInspector::SetActor(Utopian::Actor* actor)
	{
		mActor = actor;

		ClearInspectors();

		if (mActor != nullptr)
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

		CRigidBody* rigidBody = mActor->GetComponent<CRigidBody>();
		if (rigidBody != nullptr) {
			RigidBodyInspector* inspector = new RigidBodyInspector(rigidBody);
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
