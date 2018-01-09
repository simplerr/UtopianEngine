#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <vector>
#include "ShaderBuffer.h"
#include "LightData.h"

/*
	The uniform buffer that the Phong fragment shader uses
*/
class FragmentUniformBuffer : public Vulkan::ShaderBuffer
{
public:
	virtual void UpdateMemory(VkDevice device);
	virtual int GetSize();

	// Public data members
	std::vector<Vulkan::LightData> lights;

	struct {
		float numLights;
		glm::vec3 garbage;
	} constants;
};