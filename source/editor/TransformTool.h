#pragma once
#include <glm/glm.hpp>
#include "vulkan/VulkanInclude.h"
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
	class Terrain;
}

namespace Utopian
{
	class Terrain;

	//! The tool that move objects around.
	class TransformTool
	{
	public:
		TransformTool(const SharedPtr<Terrain>& terrain, Camera* camera);
		~TransformTool();

		void Update(Input* pInput, float dt);
		bool IsMovingObject();
		void SetActor(Actor* actor);

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
		SharedPtr<Renderable> mAxisX;
		SharedPtr<Renderable> mAxisY;
		SharedPtr<Renderable> mAxisZ;
		SharedPtr<Terrain> mTerrain;
		Actor* mSelectedActor;
		Camera* mCamera;
		MovingAxis	  mMovingAxis;
		glm::vec3	  mLastPlanePos;

		const float AXIS_SCALE = 180.0f;
		const float AXIS_SCALE_MAIN = 10.0f;
	};
}