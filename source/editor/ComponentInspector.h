#pragma once
#include "core/Transform.h"
#include "LightData.h"

namespace Utopian
{
	class CTransform;
	class CRenderable;
	class CLight;
	class CRigidBody;

	class ComponentInspector
	{
	public:
		ComponentInspector();

		virtual void UpdateUi() = 0;
	private:
	};

	class TransformInspector : public ComponentInspector
	{
	public:
		TransformInspector(CTransform* transform);

		virtual void UpdateUi() override;
	private:
		CTransform* mComponent;
		Transform mTransform;
	};

	class RenderableInspector : public ComponentInspector
	{
	public:
		RenderableInspector(CRenderable* renderable);

		virtual void UpdateUi() override;
	private:
		CRenderable* mRenderable;
		glm::ivec2 mTextureTiling;
		bool mDeferred;
		bool mColor;
		bool mBoundingBox;
		bool mDebugNormals;
		bool mWireframe;
	};

	class LightInspector : public ComponentInspector
	{
	public:
		LightInspector(CLight* light);

		virtual void UpdateUi() override;
	private:
		CLight* mLight;
		Utopian::LightData mLightData;
		int mType;
	};

	class RigidBodyInspector : public ComponentInspector
	{
	public:
		RigidBodyInspector(CRigidBody* rigidBody);

		virtual void UpdateUi() override;
	private:
		CRigidBody* mRigidBody;
	};
}
