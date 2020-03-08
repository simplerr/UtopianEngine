#include "ActorInspector.h"
#include "core/components/Actor.h"
#include "core/components/CLight.h"
#include "core/components/CTransform.h"
#include "core/components/CRenderable.h"
#include "core/components/CRigidBody.h"
#include "core/components/CCatmullSpline.h"
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
			if (ImGui::CollapsingHeader(mActor->GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
                if (ImGui::InputText("Name:", mActorName, IM_ARRAYSIZE(mActorName), ImGuiInputTextFlags_EnterReturnsTrue))
				{
					mActor->SetName(mActorName);
				}
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
		strcpy(mActorName, actor->GetName().c_str());

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

		CCatmullSpline* catmullSpline = mActor->GetComponent<CCatmullSpline>();
		if (catmullSpline != nullptr) {
			CatmullSplineInspector* inspector = new CatmullSplineInspector(catmullSpline);
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
