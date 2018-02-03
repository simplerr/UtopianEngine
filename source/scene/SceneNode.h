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
		
		// Setters
		void SetTransform(const Transform& transform);
		void SetPosition(const vec3& position);
		void SetRotation(const vec3& rotation);
		void SetScale(const vec3& scale);

		void AddTranslation(const vec3& translation);
		void AddRotation(const vec3& rotation);
		void AddScale(const vec3& scale);

		void SetDrawBoundingBox(bool draw);

		// Getters
		const Transform& GetTransform() const;
		const vec3& GetPosition() const;
		const vec3& GetRotation() const;
		const vec3& GetScale() const;
		const mat4& GetWorldMatrix() const;

		bool IsBoundingBoxVisible() const;
	private:
		Transform mTransform;
		bool mDrawBoundingBox;
	};
}