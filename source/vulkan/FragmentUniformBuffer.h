#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "UniformBuffer.h"
#include "Light.h"

/*
	The uniform buffer that the Phong fragment shader uses
*/
class FragmentUniformBuffer : public VulkanLib::UniformBuffer
{
public:
	virtual void UpdateMemory(VkDevice device);
	virtual int GetSize();

	// Public data members
	std::vector<VulkanLib::Light> lights;

	struct {
		float numLights;
		glm::vec3 garbage;
	} constants;
};