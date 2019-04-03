#include "ComponentInspector.h"
#include "vulkan/UIOverlay.h"
#include "vulkan/Debug.h"
#include "core/components/CTransform.h"
#include "core/components/CLight.h"
#include "core/components/CRenderable.h"
#include "core/components/Actor.h"
#include <glm/gtc/quaternion.hpp>

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

		static bool localRotate = true;
		ImGui::Checkbox("Local", &localRotate);

		glm::vec3 rotate = glm::vec3(0.0f);
		ImGui::Text("Rotation");
		ImGui::SameLine();
		ImGui::PushItemWidth(1.0f);
		ImGui::InputFloat("X", &rotate.x, 0.15);
		ImGui::SameLine();
		ImGui::InputFloat("Y", &rotate.y, 0.15);
		ImGui::SameLine();
		ImGui::InputFloat("Z", &rotate.z, 0.15);
		ImGui::PopItemWidth();

		glm::quat orientationDelta = glm::angleAxis(rotate.x, glm::vec3(1.0f, 0.0f, 0.0f));
		orientationDelta = orientationDelta * glm::angleAxis(rotate.y, glm::vec3(0.0f, 1.0f, 0.0f));
		orientationDelta = orientationDelta * glm::angleAxis(rotate.z, glm::vec3(0.0f, 0.0f, 1.0f));

		if (localRotate)
			mComponent->SetQuaternion(mComponent->GetQuaternion() * orientationDelta);
		else
			mComponent->SetQuaternion(orientationDelta * mComponent->GetQuaternion());

		mComponent->SetRotation(mTransform.GetRotation());
		mComponent->SetScale(mTransform.GetScale());
	}
}

Utopian::RenderableInspector::RenderableInspector(CRenderable* renderable)
{
	mRenderable = renderable;
	mTextureTiling = renderable->GetTextureTiling();
	mDeferred = renderable->HasRenderFlags(RenderFlags::RENDER_FLAG_DEFERRED);
	mColor = renderable->HasRenderFlags(RenderFlags::RENDER_FLAG_COLOR);
	mBoundingBox = renderable->HasRenderFlags(RenderFlags::RENDER_FLAG_BOUNDING_BOX);
	mDebugNormals = renderable->HasRenderFlags(RenderFlags::RENDER_FLAG_NORMAL_DEBUG);
	mWireframe = renderable->HasRenderFlags(RenderFlags::RENDER_FLAG_WIREFRAME);
}

void Utopian::RenderableInspector::UpdateUi()
{
	if (ImGui::CollapsingHeader("Renderable", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::Checkbox("Deferred", &mDeferred))
		{
			uint32_t flag = mRenderable->GetRenderFlags();

			if (mDeferred)
				flag |= RenderFlags::RENDER_FLAG_DEFERRED;
			else
				flag &= ~RenderFlags::RENDER_FLAG_DEFERRED;

			mRenderable->SetRenderFlags(flag);
		}

		if (ImGui::Checkbox("Color", &mColor))
		{
			uint32_t flag = mRenderable->GetRenderFlags();

			if (mColor)
				flag |= RenderFlags::RENDER_FLAG_COLOR;
			else
				flag &= ~RenderFlags::RENDER_FLAG_COLOR;

			mRenderable->SetRenderFlags(flag);
		}

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

		if (ImGui::Checkbox("Wireframe", &mWireframe))
		{
			uint32_t flag = mRenderable->GetRenderFlags();

			if (mWireframe)
				flag |= RenderFlags::RENDER_FLAG_WIREFRAME;
			else
				flag &= ~RenderFlags::RENDER_FLAG_WIREFRAME;

			mRenderable->SetRenderFlags(flag);
		}

		// If the renderable has a light then let the light inspector control the color
		glm::vec4 color = mRenderable->GetColor();
		if (!mRenderable->GetParent()->HasComponent<CLight>())
		{
			ImGui::ColorEdit3("Color", &color.x);
		}
		
		ImGui::SliderFloat("Brightness", &color.w, 0.0f, 100.0, "%.1f");
		mRenderable->SetColor(color);

		ImGui::SliderInt("Tiling", &mTextureTiling.x, 1, 200);

		mRenderable->SetTileFactor(glm::vec2(mTextureTiling.x, mTextureTiling.y));
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
		ImGui::SliderFloat("Range", &mLightData.range, 0.0f, 10000.0f);
		ImGui::SliderFloat("Spot", &mLightData.spot, 0.0f, 30.0f);

		ImGui::ColorEdit3("Color", &mLightData.material.ambient.x);
		mLightData.material.diffuse = mLightData.material.ambient;
		mLightData.material.specular = mLightData.material.ambient;

		mLight->SetIntensity(mLightData.intensity);
		mLight->SetDirection(mLightData.direction);
		mLight->SetMaterial(mLightData.material);
		mLight->SetRange(mLightData.range);
		mLight->SetSpot(mLightData.spot);
		mLight->SetAttenuation(mLightData.att);
		mLight->SetType((Utopian::LightType)mType);
	}
}
