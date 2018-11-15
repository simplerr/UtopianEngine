#include "ComponentInspector.h"
#include "vulkan/UIOverlay.h"
#include "core/components/CTransform.h"
#include "core/components/CLight.h"
#include "core/components/CRenderable.h"

Utopian::ComponentInspector::ComponentInspector()
{
}

Utopian::TransformInspector::TransformInspector(CTransform* transform)
{
	mComponent = transform;
	mTransform = mComponent->GetTransform();
}

void Utopian::TransformInspector::UpdateUi()
{
	mTransform = mComponent->GetTransform();

	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::InputFloat3("Position", &mTransform.mPosition.x, 2);
		ImGui::SliderFloat3("Scale", &mTransform.mScale.x, 0.0f, 100.0f, "%.1f");
		ImGui::SliderFloat3("Rotation", &mTransform.mRotation.x, 0.0f, 360.0f, "%.1f");

		mComponent->SetRotation(mTransform.GetRotation());
		mComponent->SetScale(mTransform.GetScale());
	}
}

Utopian::RenderableInspector::RenderableInspector(CRenderable* renderable)
{
	mRenderable = renderable;
	mBoundingBox = renderable->HasRenderFlags(RenderFlags::RENDER_FLAG_BOUNDING_BOX);
	mDebugNormals = renderable->HasRenderFlags(RenderFlags::RENDER_FLAG_NORMAL_DEBUG);
}

void Utopian::RenderableInspector::UpdateUi()
{
	if (ImGui::CollapsingHeader("Renderable", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::Checkbox("Bounding box", &mBoundingBox))
		{
			uint32_t flag = mRenderable->GetRenderFlags();

			if (mBoundingBox)
				flag |= RenderFlags::RENDER_FLAG_BOUNDING_BOX;
			else
				flag &= ~RenderFlags::RENDER_FLAG_BOUNDING_BOX;

			mRenderable->SetRenderFlags(flag);
		}

		if (ImGui::Checkbox("Debug normals", &mDebugNormals))
		{
			uint32_t flag = mRenderable->GetRenderFlags();

			if (mDebugNormals)
				flag |= RenderFlags::RENDER_FLAG_NORMAL_DEBUG;
			else
				flag &= ~RenderFlags::RENDER_FLAG_NORMAL_DEBUG;

			mRenderable->SetRenderFlags(flag);
		}
	}
}

Utopian::LightInspector::LightInspector(CLight* light)
{
	mLight = light;
	mLightData = mLight->GetLightData();
	mType = light->GetLightType();
}

void Utopian::LightInspector::UpdateUi()
{
	if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Combo("Type", &mType, "Directional\0Point\0Spot\0");

		ImGui::SliderFloat3("Intensity", &mLightData.intensity.x, 0.0f, 1.0f);
		ImGui::SliderFloat3("Direction", &mLightData.direction.x, -1.0f, 1.0f);
		ImGui::SliderFloat3("Attenuation", &mLightData.att.x, 0.0f, 3.0f);
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
		mLight->SetAttenuation(mLightData.att);
		mLight->SetType((Utopian::Vk::LightType)mType);
	}
}
