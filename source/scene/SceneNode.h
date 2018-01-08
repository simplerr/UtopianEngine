#pragma once
#include "scene/Transform.h"

namespace Scene
{
	/*
		Represents a node in the scene graph.
		Inherited by Light, Renderable, ParticleSystem etc.
	*/
	class SceneNode
	{
	public:
		SceneNode();
		~SceneNode();
		
		void SetTransform(const Transform& transform);

		const Transform& GetTransform() const;
	private:
		Transform mTransform;

	};
}