#include "TransformTool.h"
#include "Camera.h"
#include <limits>
#include "Effects.h"
#include "Terrain.h"
#include "Input.h"
#include "vulkan/VulkanDebug.h"
#include <glm/gtx/intersect.hpp>
#include "ecs/systems/System.h"
#include "ecs/components/TransformComponent.h"
#include "scene/Actor.h"
#include "scene/CTransform.h"

#define GLM_FORCE_RIGHT_HANDED 

TransformTool::TransformTool(Vulkan::Camera* camera, Terrain* terrain)
{
	// nullptr as default.
	mSelectedActor = nullptr;
	mCamera = camera;
	mTerrain = terrain;

	//// Create the X axis.
	//mAxisX = new StaticObject(pImporter, "models/arrow.obj");
	//mAxisX->SetPosition(vec3(0, 30, 30));
	//mAxisX->SetMaterials(Material(Colors::Green));
	//mAxisX->SetRotation(vec3(3.14f / 2.0f, 3.14f / 2.0f, 0));
	//mAxisX->SetScale(vec3(1.50f, 1.50f, 1.50f));

	//// Create the Y axis.
	//mAxisY = new StaticObject(pImporter, "models/arrow.obj");
	//mAxisY->SetPosition(vec3(0, 30, 30));
	//mAxisY->SetMaterials(Material(Colors::Red));
	//mAxisY->SetRotation(vec3(0, 1, 0));
	//mAxisY->SetScale(vec3(1.50f, 1.50f, 1.50f));

	//// Create the Z axis.
	//mAxisZ = new StaticObject(pImporter, "models/arrow.obj");
	//mAxisZ->SetPosition(vec3(0, 30, 30));
	//mAxisZ->SetMaterials(Material(Colors::Blue));
	//mAxisZ->SetRotation(vec3(0, 3.14f / 2.0f, 3.14f / 2.0f));
	//mAxisZ->SetScale(vec3(1.50f, 1.50f, 1.50f));

	mMovingAxis = NONE;
}

//! Cleanup.
TransformTool::~TransformTool()
{
	delete mAxisX;
	delete mAxisY;
	delete mAxisZ;
}

void TransformTool::InitStartingPosition(Input* pInput, vec3& dir, vec3& cameraPos, float& dist)
{
	dist = std::numeric_limits<float>::infinity();
	cameraPos = mCamera->GetPosition();
	dir = mCamera->GetPickingRay().direction;

	// Store as the last plane pos.
	mLastPlanePos = cameraPos + dir * dist;
	mLastPlanePos = vec3(std::numeric_limits<float>::infinity());
}

//! Poll for input and perform actions.
void TransformTool::Update(Input* pInput, float dt)
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

		// Find out which axis arrow was pressed.
		/*if (mAxisX->RayIntersect(XMLoadFloat3(&pos), XMLoadFloat3(&dir), dist))
			mMovingAxis = X_AXIS;
		else if (mAxisY->RayIntersect(XMLoadFloat3(&pos), XMLoadFloat3(&dir), dist))
			mMovingAxis = Y_AXIS;
		else if (mAxisZ->RayIntersect(XMLoadFloat3(&pos), XMLoadFloat3(&dir), dist))
			mMovingAxis = Z_AXIS;*/
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

		Vulkan::VulkanDebug::ConsolePrint(dir, "picking dir:");
	}

	// Stick to the terain?
	if (pInput->KeyPressed('C')) {
		Scene::CTransform* transform = mSelectedActor->GetComponent<Scene::CTransform>();
		vec3 position = transform->GetPosition();
		transform->SetPosition(glm::vec3(position.x, mTerrain->GetHeight(position.x, position.z), position.z));
		//float height = mMovingObject->GetWorld()->GetTerrain()->GetHeight(mMovingObject->GetPosition().x, mMovingObject->GetPosition().z);
		//if (height != -std::numeric_limits<float>::infinity())
		//	UpdatePosition(vec3(mMovingObject->GetPosition().x, height, mMovingObject->GetPosition().z) - mMovingObject->GetPosition());
	}
}

//! Draws the arrow axis.
void TransformTool::Draw()
{
	// Disable the depth test.
	//ID3D11DepthStencilState* oldState = nullptr;
	//pGraphics->GetContext()->OMGetDepthStencilState(&oldState, 0);
	//pGraphics->GetContext()->OMSetDepthStencilState(RenderStates::EnableAllDSS, 0);

	//Effects::BasicFX->SetUseLighting(false);

	//// The axes will be rendered through the object.
	//mAxisX->Draw(pGraphics);
	//mAxisY->Draw(pGraphics);
	//mAxisZ->Draw(pGraphics);

	//Effects::BasicFX->SetUseLighting(true);

	//// Restore to standard depth stencil state (enable depth testing).
	//pGraphics->GetContext()->OMSetDepthStencilState(oldState, 0);

	//XMMATRIX view = XMLoadFloat4x4(&pGraphics->GetCamera()->GetViewMatrix());
	//XMMATRIX proj = XMLoadFloat4x4(&pGraphics->GetCamera()->GetProjectionMatrix());
}

//! Updates the moving objects position.
void TransformTool::UpdatePosition(vec3 delta)
{
	if (mSelectedActor != nullptr) {
		Scene::CTransform* transform = mSelectedActor->GetComponent<Scene::CTransform>();
		transform->AddTranslation(delta);
		//onPositionChange(mMovingObject->GetPosition());
	}

	//mAxisX->SetPosition(mAxisX->GetPosition() + delta);
	//mAxisY->SetPosition(mAxisY->GetPosition() + delta);
	//mAxisZ->SetPosition(mAxisZ->GetPosition() + delta);
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
	bool intersection = intersectPlane(vec3(0.0f, -1.0f, 0.0f), vec3(0.0f), pos, dir, dist);

	float dx;
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
	//// Top right triangle.
	//vec3 right = GetCamera()->GetRight();
	//vec3 up = vec3(0, 1, 0);
	//vec3 objectPos = mAxisY->GetPosition();
	//float halfWidth = 60.0f;

	////mLastPlanePos - 1000*XMLoadFloat3(&right);
	//vec3 v0 = vec3(objectPos + right*halfWidth + up*halfWidth);
	//vec3 v1 = vec3(objectPos - right*halfWidth + up*halfWidth);
	//vec3 v2 = vec3(objectPos - right*halfWidth - up*halfWidth);
	//float dist = std::numeric_limits<float>::infinity();
	//if (!XNA::IntersectRayTriangle(XMLoadFloat3(&pos), XMLoadFloat3(&dir), v0, v1, v2, &dist))
	//{
	//	// Bottom left triangle.
	//	v0 = vec3(objectPos + right*halfWidth + up*halfWidth);
	//	v1 = vec3(objectPos - right*halfWidth - up*halfWidth);
	//	v2 = vec3(objectPos + right*halfWidth - up*halfWidth);
	//	dist = std::numeric_limits<float>::infinity();
	//	XNA::IntersectRayTriangle(XMLoadFloat3(&pos), XMLoadFloat3(&dir), v0, v1, v2, &dist);
	//}

	//float dy;
	//if (mLastPlanePos.x != std::numeric_limits<float>::infinity()) {
	//	vec3 planePos = pos + dir * dist;
	//	dy = planePos.y - mLastPlanePos.y;
	//	mLastPlanePos = planePos;
	//}
	//else {
	//	mLastPlanePos = pos + dir * dist;
	//	dy = 0.0f;
	//}

	//return vec3(0, dy, 0);

	return vec3(0, 0, 0);
}

//! Move the object on the Z axis.
vec3 TransformTool::MoveAxisZ(vec3 pos, vec3 dir)
{
	float dist = std::numeric_limits<float>::infinity();
	bool intersection = intersectPlane(vec3(0.0f, -1.0f, 0.0f), vec3(0.0f), pos, dir, dist);

	float dz;
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
	//vec3 diff = GLib::GetCamera()->GetPosition() - mMovingObject->GetPosition();
	//float dist = sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
	//float scale = dist / 60.0f;
	//mAxisX->SetScale(vec3(scale, scale, scale));
	//mAxisY->SetScale(vec3(scale, scale, scale));
	//mAxisZ->SetScale(vec3(scale, scale, scale));
	//SetPosition(mMovingObject->GetPosition());
}

//! Sets the axis positions.
void TransformTool::SetPosition(vec3 position)
{
	//mAxisX->SetPosition(position + vec3(mAxisX->GetBoundingBox().Extents.x*0.81, 0, 0));
	//mAxisY->SetPosition(position + vec3(0, mAxisY->GetBoundingBox().Extents.y*0.81, 0));
	//mAxisZ->SetPosition(position + vec3(0, 0, mAxisZ->GetBoundingBox().Extents.z*0.81));
}

bool TransformTool::IsMovingObject()
{
	return mMovingAxis == NONE ? false : true;
}

void TransformTool::SetActor(Scene::Actor* actor)
{
	mSelectedActor = actor;

	//mAxisX->SetPosition(pObject->GetPosition());

	//SetPosition(pObject->GetPosition());
}
