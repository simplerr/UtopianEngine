#pragma once
#include "scene/Transform.h"
#include "LightData.h"

namespace Scene
{
	class CTransform;
	class CRenderable;
	class CLight;

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
	};

	class LightInspector : public ComponentInspector
	{
	public:
		LightInspector(CLight* light);

		virtual void UpdateUi() override;
	private:
		CLight* mLight;
		Vulkan::LightData mLightData;
		int mType;
	};
}