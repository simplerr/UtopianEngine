#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <string>

using namespace glm;

// Temporary defines to not rotate the sky sphere and terrain
#define OBJECT_ID_SKY 1
#define OBJECT_ID_TERRAIN 2
#define OBJECT_ID_PROP 3

enum PipelineEnum {
	TEXTURED, COLORED, STARSPHERE
};

namespace VulkanLib
{
	class StaticModel;

	class Object
	{
	public:
		Object(vec3 position);
		~Object();

		void SetModel(std::string modelSource);
		void SetPosition(vec3 position);
		void SetRotation(vec3 rotation);
		void SetScale(vec3 scale);
		void SetColor(vec3 color);
		void SetId(int id);

		void AddRotation(float x, float y, float z);

		void SetPipeline(PipelineEnum pipeline);


		std::string GetModel();
		vec3 GetPosition();
		vec3 GetRotation();
		vec3 GetScale();
		vec3 GetColor();
		mat4 GetWorldMatrix();
		mat4 GetWorldInverseTransposeMatrix();
		int GetId();

		PipelineEnum GetPipeline();
	private:
		void RebuildWorldMatrix();

		//	StaticModel* mModel;
		std::string mModelSource;
		mat4 mWorld;
		vec3 mPosition;
		vec3 mRotation;
		vec3 mScale;
		vec3 mColor;
		int mId;

		PipelineEnum mPipeline;
	};
}	// VulkanLib namespace