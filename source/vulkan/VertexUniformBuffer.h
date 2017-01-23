#pragma once

#include <glm/glm.hpp>
#include "UniformBuffer.h"
#include "Light.h"

/*
The big uniform buffer that contains everything
*/
namespace VulkanLib
{
	class VertexUniformBuffer : public UniformBuffer
	{
	public:
		virtual void UpdateMemory(VkDevice device);
		virtual int GetSize();

		// Public data members
		struct {
			glm::mat4 projectionMatrix;
			glm::mat4 viewMatrix;
			glm::vec4 lightDir = glm::vec4(1.0f, -1.0f, 1.0f, 1.0f);
			glm::vec3 eyePos;
			float t;
		} camera;

		struct {
			bool useInstancing;
			glm::vec3 garbage;
		} constants; // Currently unused
	};
}