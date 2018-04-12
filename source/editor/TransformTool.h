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

namespace Utopian
{
	class Actor;
	class Renderable;
	class Input;
}

class Terrain;

//! The tool that move objects around.
class TransformTool
{
public:
	TransformTool(Utopian::Vk::Renderer* renderer, Terrain* terrain);
	~TransformTool();

	void Update(Utopian::Input* pInput, float dt);
	void Draw(Utopian::Vk::CommandBuffer* commandBuffer);
	bool IsMovingObject();
	void SetActor(Utopian::Actor* actor);

	// These are only supposed to be called by an BaseInspector type.
	void SetPosition(glm::vec3 position);

private:
	glm::vec3 MoveAxisX(glm::vec3 pos, glm::vec3 dir);
	glm::vec3 MoveAxisY(glm::vec3 pos, glm::vec3 dir);
	glm::vec3 MoveAxisZ(glm::vec3 pos, glm::vec3 dir);
	void UpdatePosition(glm::vec3 delta);
	void InitStartingPosition(Utopian::Input* pInput, glm::vec3& dir, glm::vec3& cameraPos, float& dist);
	void ScaleAxisArrows();

private:
	SharedPtr<Utopian::Renderable> mAxisX;
	SharedPtr<Utopian::Renderable> mAxisY;
	SharedPtr<Utopian::Renderable> mAxisZ;
	Utopian::Vk::ColorEffect mColorEffect;
	Utopian::Actor* mSelectedActor;
	Utopian::Camera* mCamera;
	Terrain* mTerrain;
	MovingAxis	  mMovingAxis;
	glm::vec3	  mLastPlanePos;

	const float AXIS_SCALE = 90.0f;
	const float AXIS_SCALE_MAIN = 10.0f;
	const float PLANE_SIZE = 100000.0;
};