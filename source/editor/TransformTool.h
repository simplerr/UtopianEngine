#pragma once
#include <glm/glm.hpp>
#include "vulkan/ColorEffect.h"
#include "utility/Common.h"

enum MovingAxis
{
	X_AXIS,
	Y_AXIS,
	Z_AXIS,
	XZ_AXIS,
	NONE
};

namespace Vulkan
{
	class StaticModel;
	class Camera;
}

namespace ECS
{
	class EntityCache;
}

namespace Scene
{
	class Actor;
	class Renderable;
}

class Input;
class Terrain;

//! The tool that move objects around.
class TransformTool
{
public:
	TransformTool(Vulkan::Renderer* renderer, Terrain* terrain);
	~TransformTool();

	void Update(Input* pInput, float dt);
	void Draw(Vulkan::CommandBuffer* commandBuffer);
	bool IsMovingObject();
	void SetActor(Scene::Actor* actor);

	// These are only supposed to be called by an BaseInspector type.
	void SetPosition(glm::vec3 position);

private:
	glm::vec3 MoveAxisX(glm::vec3 pos, glm::vec3 dir);
	glm::vec3 MoveAxisY(glm::vec3 pos, glm::vec3 dir);
	glm::vec3 MoveAxisZ(glm::vec3 pos, glm::vec3 dir);
	void UpdatePosition(glm::vec3 delta);
	void InitStartingPosition(Input* pInput, glm::vec3& dir, glm::vec3& cameraPos, float& dist);
	void ScaleAxisArrows();

private:
	SharedPtr<Scene::Renderable> mAxisX;
	SharedPtr<Scene::Renderable> mAxisY;
	SharedPtr<Scene::Renderable> mAxisZ;
	Scene::Actor* mSelectedActor;
	MovingAxis	  mMovingAxis;
	glm::vec3	  mLastPlanePos;
	Vulkan::Camera* mCamera;
	Terrain* mTerrain;
	Vulkan::ColorEffect mColorEffect;

	const float AXIS_SCALE = 30.0f;
	const float PLANE_SIZE = 100000.0;
};