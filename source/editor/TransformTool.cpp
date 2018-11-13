#include "TransformTool.h"
#include "Camera.h"
#include <limits>
#include "Effects.h"
#include "core/terrain/Terrain.h"
#include "Input.h"
#include "vulkan/VulkanDebug.h"
#include <glm/gtx/intersect.hpp>
#include "core/components/Actor.h"
#include "core/components/CTransform.h"
#include "vulkan/Renderer.h"
#include "vulkan/handles/CommandBuffer.h"
#include "vulkan/ModelLoader.h"
#include "core/renderer/Renderable.h"
#include "core/renderer/RenderingManager.h"
#include "utility/math/Ray.h"
#include "Colors.h"

using namespace Utopian;
using namespace Utopian::Vk;

#define GLM_FORCE_RIGHT_HANDED 

TransformTool::TransformTool(Utopian::Vk::Renderer* renderer, Terrain* terrain)
{
	// nullptr as default.
	mSelectedActor = nullptr;
	mCamera = renderer->GetCamera();
	mTerrain = terrain;

	mAxisX = Utopian::Renderable::Create();
	mAxisX->SetScale(vec3(AXIS_SCALE * AXIS_SCALE_MAIN, AXIS_SCALE, AXIS_SCALE));
	mAxisX->SetColor(Utopian::Vk::Colors::Red);
	mAxisX->SetMaterial(Mat(EffectType::COLOR, PhongEffect::NORMAL));
	mAxisX->LoadModel("data/models/cube.obj");
	mAxisX->SetVisible(false);

	mAxisY = Utopian::Renderable::Create();
	mAxisY->SetScale(vec3(AXIS_SCALE, AXIS_SCALE * AXIS_SCALE_MAIN, AXIS_SCALE));
	mAxisY->SetColor(Utopian::Vk::Colors::Green);
	mAxisY->SetMaterial(Mat(EffectType::COLOR, PhongEffect::NORMAL));
	mAxisY->LoadModel("data/models/cube.obj");
	mAxisY->SetVisible(false);

	mAxisZ = Utopian::Renderable::Create();
	mAxisZ->SetScale(vec3(AXIS_SCALE, AXIS_SCALE, AXIS_SCALE * AXIS_SCALE_MAIN));
	mAxisZ->SetColor(Utopian::Vk::Colors::Blue);
	mAxisZ->SetMaterial(Mat(EffectType::COLOR, PhongEffect::NORMAL));
	mAxisZ->LoadModel("data/models/cube.obj");
	mAxisZ->SetVisible(false);

	mMovingAxis = NONE;

	mColorEffect.Init(renderer);
}

//! Cleanup.
TransformTool::~TransformTool()
{
}

void TransformTool::InitStartingPosition(Utopian::Input* pInput, vec3& dir, vec3& cameraPos, float& dist)
{
	dist = std::numeric_limits<float>::infinity();
	cameraPos = mCamera->GetPosition();
	dir = mCamera->GetPickingRay().direction;

	// Store as the last plane pos.
	mLastPlanePos = cameraPos + dir * dist;
	mLastPlanePos = vec3(std::numeric_limits<float>::infinity());
}

//! Poll for input and perform actions.
void TransformTool::Update(Utopian::Input* pInput, float dt)
{
	if (mSelectedActor == nullptr)
		return;

	float dist;
	vec3 pos, dir;

	// Scale the axis arrows.
	ScaleAxisArrows();

	// LBUTTON pressed and inside 3D screen.
	if (pInput->KeyPressed(VK_LBUTTON))
	{
		InitStartingPosition(pInput, dir, pos, dist);

		Utopian::Ray ray = mCamera->GetPickingRay();
		float distance = FLT_MAX;

		Utopian::BoundingBox boundingBoxX = mAxisX->GetBoundingBox();
		Utopian::BoundingBox boundingBoxY = mAxisY->GetBoundingBox();
		Utopian::BoundingBox boundingBoxZ = mAxisZ->GetBoundingBox();

		if (boundingBoxX.RayIntersect(ray, distance))
		{
			mMovingAxis = X_AXIS;
		}
		else if (boundingBoxY.RayIntersect(ray, distance))
		{
			mMovingAxis = Y_AXIS;
		}
		else if (boundingBoxZ.RayIntersect(ray, distance))
		{
			mMovingAxis = Z_AXIS;
		}
	}
	else if (pInput->KeyPressed(VK_RBUTTON)) {
		InitStartingPosition(pInput, dir, pos, dist);

		//if (mMovingObject->RayIntersect(pos, dir, dist))
			mMovingAxis = XZ_AXIS;
	}

	// Not moving any axis any more.
	if (pInput->KeyReleased(VK_LBUTTON) || pInput->KeyReleased(VK_RBUTTON))
		mMovingAxis = NONE;

	// Update the position.
	if (mMovingAxis != NONE)
	{
		vec3 pos = mCamera->GetPosition();
		vec3 dir = mCamera->GetPickingRay().direction;

		if (mMovingAxis == X_AXIS)
			UpdatePosition(MoveAxisX(pos, dir));
		else if (mMovingAxis == Y_AXIS)
			UpdatePosition(MoveAxisY(pos, dir));
		else if (mMovingAxis == Z_AXIS)
			UpdatePosition(MoveAxisZ(pos, dir));
	}

	// Scaling with CTRL + mwheel.
	//if (pInput->KeyDown(VK_CONTROL) && mMovingObject->GetType() != LIGHT_OBJECT) {
	//	mMovingObject->SetScale(mMovingObject->GetScale() + vec3(pInput->MouseDz() / 1300.0f, pInput->MouseDz() / 1300.0f, pInput->MouseDz() / 1300.0f));
	//	onScaleChange(mMovingObject->GetScale());
	//}

	// Move on terrain with RBUTTON.
	if (pInput->KeyDown(VK_RBUTTON) && mMovingAxis == XZ_AXIS)
	{
		vec3 pos = mCamera->GetPosition();
		vec3 dir = mCamera->GetPickingRay().direction;

		UpdatePosition(MoveAxisZ(pos, dir));
		UpdatePosition(MoveAxisX(pos, dir));

		Utopian::Vk::VulkanDebug::ConsolePrint(dir, "picking dir:");
	}

	// Stick to the terain?
	if (pInput->KeyPressed('T')) {
		Utopian::CTransform* transform = mSelectedActor->GetComponent<Utopian::CTransform>();
		vec3 position = transform->GetPosition();
		transform->SetPosition(glm::vec3(position.x, mTerrain->GetHeight(position.x, position.z), position.z));
	}
}

//! Updates the moving objects position.
void TransformTool::UpdatePosition(vec3 delta)
{
	if (mSelectedActor != nullptr) {
		Utopian::CTransform* transform = mSelectedActor->GetComponent<Utopian::CTransform>();
		transform->AddTranslation(delta);
	}

	mAxisX->SetPosition(mAxisX->GetPosition() + delta);
	mAxisY->SetPosition(mAxisY->GetPosition() + delta);
	mAxisZ->SetPosition(mAxisZ->GetPosition() + delta);
}

bool intersectPlane(vec3 n, vec3 p0, vec3 origin, vec3 dir, float &t)
{
	// assuming vectors are all normalized
	float denom = glm::dot(n, dir);
	if (denom > 1e-6) {
		vec3 p0l0 = p0 - origin;
		t = glm::dot(p0l0, n) / denom;
		return (t >= 0);
	}

	return false;
}

//! Move the object on the X axis.
vec3 TransformTool::MoveAxisX(vec3 pos, vec3 dir)
{
	float dist = std::numeric_limits<float>::infinity();
	float y = mSelectedActor->GetTransform().GetPosition().y;
	bool intersection = intersectPlane(vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, y, 0.0f), pos, dir, dist);

	float dx = 0;
	if (intersection)
	{
		if (mLastPlanePos.x != std::numeric_limits<float>::infinity()) {
			vec3 planePos = pos + dir * dist;
			dx = planePos.x - mLastPlanePos.x;
			mLastPlanePos.x = planePos.x;
		}
		else {
			mLastPlanePos = pos + dir * dist;
			dx = 0.0f;
		}
	}
	
	return vec3(dx, 0, 0);
}

//! Move the object on the Y axis.
vec3 TransformTool::MoveAxisY(vec3 pos, vec3 dir)
{
	float dist = std::numeric_limits<float>::infinity();
	float x = mSelectedActor->GetTransform().GetPosition().x;
	float z = mSelectedActor->GetTransform().GetPosition().z;
	bool intersection = intersectPlane(glm::vec3(dir.x, 0.0f, dir.z), vec3(x, 0.0f, z), pos, dir, dist);

	float dy = 0;
	if (intersection)
	{
		if (mLastPlanePos.y != std::numeric_limits<float>::infinity()) {
			vec3 planePos = pos + dir * dist;
			dy = planePos.y - mLastPlanePos.y;
			mLastPlanePos.y = planePos.y;
		}
		else {
			mLastPlanePos = pos + dir * dist;
			dy = 0.0f;
		}
	}
	
	return vec3(0, dy, 0);
}

//! Move the object on the Z axis.
vec3 TransformTool::MoveAxisZ(vec3 pos, vec3 dir)
{
	float dist = std::numeric_limits<float>::infinity();
	float y = mSelectedActor->GetTransform().GetPosition().y;
	bool intersection = intersectPlane(vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, y, 0.0f), pos, dir, dist);

	float dz = 0;
	if (intersection)
	{
		if (mLastPlanePos.z != std::numeric_limits<float>::infinity()) {
			vec3 planePos = pos + dir * dist;
			dz = planePos.z - mLastPlanePos.z;
			mLastPlanePos.z = planePos.z;
		}
		else {
			mLastPlanePos = pos + dir * dist;
			dz = 0.0f;
		}
	}
	
	return vec3(0, 0, dz);
}

//! Scales the axis arrows so they allways have the same size on the screen.
void TransformTool::ScaleAxisArrows()
{
	vec3 diff = mCamera->GetPosition() - mSelectedActor->GetTransform().GetPosition();
	float dist = sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
	float scale = dist / AXIS_SCALE;
	mAxisX->SetScale(vec3(scale * AXIS_SCALE_MAIN, scale, scale));
	mAxisY->SetScale(vec3(scale, scale * AXIS_SCALE_MAIN, scale));
	mAxisZ->SetScale(vec3(scale, scale, scale  * AXIS_SCALE_MAIN));
	SetPosition(mSelectedActor->GetTransform().GetPosition());
}

//! Sets the axis positions.
void TransformTool::SetPosition(vec3 position)
{
	float offset = 1.00f;
	mAxisX->SetPosition(position + vec3(mAxisX->GetBoundingBox().GetWidth() / 2.0f * offset, 0, 0));
	mAxisY->SetPosition(position + vec3(0, mAxisY->GetBoundingBox().GetHeight() / 2.0f * offset, 0));
	mAxisZ->SetPosition(position + vec3(0, 0, -mAxisZ->GetBoundingBox().GetDepth() / 2.0f * offset));
}

bool TransformTool::IsMovingObject()
{
	return mMovingAxis == NONE ? false : true;
}

void TransformTool::SetActor(Utopian::Actor* actor)
{
	mSelectedActor = actor;

	if (actor != nullptr)
	{
		SetPosition(mSelectedActor->GetTransform().GetPosition());
		ScaleAxisArrows();

		mAxisX->SetVisible(true);
		mAxisY->SetVisible(true);
		mAxisZ->SetVisible(true);
	}
	else
	{
		mAxisX->SetVisible(false);
		mAxisY->SetVisible(false);
		mAxisZ->SetVisible(false);
	}
}
