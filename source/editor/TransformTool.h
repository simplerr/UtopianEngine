#pragma once
#include <glm/glm.hpp>

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
}

class Input;
class Terrain;

//! The tool that move objects around.
class TransformTool
{
public:
	TransformTool(Vulkan::Camera* camera, Terrain* terrain);
	~TransformTool();

	void Update(Input* pInput, float dt);
	void Draw();
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
	Vulkan::StaticModel* mAxisX;
	Vulkan::StaticModel* mAxisY;
	Vulkan::StaticModel* mAxisZ;
	Scene::Actor* mSelectedActor;
	MovingAxis	  mMovingAxis;
	glm::vec3	  mLastPlanePos;
	Vulkan::Camera* mCamera;
	Terrain* mTerrain;

	const float PLANE_SIZE = 100000.0;
};