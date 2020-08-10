#pragma once
#include "PhysicsDebugDraw.h"
#include "BulletHelpers.h"
#include "btBulletDynamicsCommon.h"
#include "im3d/im3d.h"
#include "core/Log.h"

namespace Utopian
{
	PhysicsDebugDraw::PhysicsDebugDraw()
	{
		mDebugMode = DBG_DrawWireframe | DBG_DrawContactPoints | DBG_DrawConstraints | DBG_DrawConstraintLimits | DBG_DrawNormals | DBG_DrawFrames;
		mDebugMode = DBG_DrawAabb; // Others don't work when having a btHeightfieldTerrainShape
	}

	PhysicsDebugDraw::~PhysicsDebugDraw()
	{

	}

	void PhysicsDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor)
	{
		Im3d::Vec4 color = Im3d::Vec4(ToVec3(fromColor), 1.0f);
		Im3d::DrawLine(ToVec3(from), ToVec3(to), 3.0f, Im3d::Color(color));
	}

	void PhysicsDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
	{
		drawLine(from, to, color, color);
	}

	void PhysicsDebugDraw::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
	{
		distance = 10.0f; // Hardcoded since the supplied distance is extremely small
		const btVector3& from = PointOnB;
		btVector3 to = PointOnB + normalOnB * distance;
		drawLine(from, to, color);
	}

	void PhysicsDebugDraw::drawTransform(const btTransform& transform, btScalar orthoLen)
	{
		orthoLen = 10.0f; // Hardcoded since the supplied orthoLen is extremely small
		btVector3 start = transform.getOrigin();
		drawLine(start, start + transform.getBasis() * btVector3(orthoLen, 0, 0), btVector3(btScalar(1.), btScalar(0.3), btScalar(0.3)));
		drawLine(start, start + transform.getBasis() * btVector3(0, orthoLen, 0), btVector3(btScalar(0.3), btScalar(1.), btScalar(0.3)));
		drawLine(start, start + transform.getBasis() * btVector3(0, 0, orthoLen), btVector3(btScalar(0.3), btScalar(0.3), btScalar(1.)));
	}

	void PhysicsDebugDraw::reportErrorWarning(const char* warningString)
	{
		UTO_LOG(warningString);
	}

	void PhysicsDebugDraw::draw3dText(const btVector3& location, const char* textString) 
	{
		// Not implemented
	}

	void PhysicsDebugDraw::setDebugMode(int debugMode) 
	{
		mDebugMode = debugMode;
	}

	int PhysicsDebugDraw::getDebugMode() const
	{
		return mDebugMode;
	}
}