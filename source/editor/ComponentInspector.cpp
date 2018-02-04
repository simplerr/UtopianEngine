#include "ComponentInspector.h"
#include "vulkan/UIOverlay.h"
#include "scene/CTransform.h"
#include "scene/CLight.h"

Scene::ComponentInspector::ComponentInspector()
{
}

Scene::TransformInspector::TransformInspector(CTransform* transform)
{
	mComponent = transform;
	mTransform = mComponent->GetTransform();
}

void Scene::TransformInspector::UpdateUi()
{
	mTransform = mComponent->GetTransform();

	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::InputFloat3("Position", &mTransform.mPosition.x, 2);
		ImGui::SliderFloat3("Scale", &mTransform.mScale.x, 0.0f, 2000.0f, "%.1f");
		ImGui::SliderFloat3("Rotation", &mTransform.mRotation.x, 0.0f, 360.0f, "%.1f");

		mComponent->SetRotation(mTransform.GetRotation());
		mComponent->SetScale(mTransform.GetScale());
	}
}

Scene::RenderableInspector::RenderableInspector(CRenderable* renderable)
{
	mRenderable = renderable;
}

void Scene::RenderableInspector::UpdateUi()
{
	if (ImGui::CollapsingHeader("Renderable", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Mesh data [todo]");
	}
}

Scene::LightInspector::LightInspector(CLight* light)
{
	mLight = light;
	mLightData = mLight->GetLightData();
	mType = light->GetLightType();
}

void Scene::LightInspector::UpdateUi()
{
	if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Combo("Type", &mType, "Directional\0Point\0Spot\0");

		ImGui::SliderFloat3("Intensity", &mLightData.intensity.x, 0.0f, 1.0f);
		ImGui::SliderFloat3("Direction", &mLightData.direction.x, -1.0f, 1.0f);
		ImGui::SliderFloat3("Attenuation", &mLightData.att.x, 0.0f, 1.0f);
		ImGui::SliderFloat("Range", &mLightData.range, 0.0f, 100000.0f);
		ImGui::SliderFloat("Spot", &mLightData.spot, 0.0f, 100000.0f);

		ImGui::ColorEdit4("Ambient color", &mLightData.material.ambient.x);
		ImGui::ColorEdit4("Diffuse color", &mLightData.material.diffuse.x);
		ImGui::ColorEdit4("Specular color", &mLightData.material.specular.x);

		mLight->SetIntensity(mLightData.intensity);
		mLight->SetDirection(mLightData.direction);
		mLight->SetMaterial(mLightData.material);
		mLight->SetRange(mLightData.range);
		mLight->SetSpot(mLightData.spot);
		mLight->SetType((Vulkan::LightType)mType);
	}
}
