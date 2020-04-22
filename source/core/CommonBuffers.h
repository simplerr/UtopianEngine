#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"
#include "LightData.h"

namespace Utopian
{
	class CameraUniformBuffer : public Utopian::Vk::ShaderBuffer
	{
	public:
		virtual void UpdateMemory()
		{
			// Map uniform buffer and update it
			uint8_t *mapped;
			mBuffer->MapMemory(0, sizeof(camera), 0, (void**)&mapped);
			memcpy(mapped, &camera, sizeof(camera));
			mBuffer->UnmapMemory();
		}

		virtual int GetSize()
		{
			return sizeof(camera) + sizeof(constants);
		}

		// Public data members
		struct {
			glm::mat4 projectionMatrix;
			glm::mat4 viewMatrix;
			glm::vec4 clippingPlane;
			glm::vec3 eyePos;
			float t;
		} camera;

		struct {
			bool useInstancing;
			glm::vec3 garbage;
		} constants; // Currently unused
	};

	class LightUniformBuffer : public Utopian::Vk::ShaderBuffer
	{
	public:
		virtual void UpdateMemory()
		{
			// Map and update the light data
			uint8_t* mapped;
			uint32_t dataOffset = 0;
			uint32_t dataSize = sizeof(constants);
			mBuffer->MapMemory(dataOffset, dataSize, 0, (void**)&mapped);
			memcpy(mapped, &constants.numLights, dataSize);
			mBuffer->UnmapMemory();

			// Map and update number of lights
			dataOffset += dataSize;
			dataSize = lights.size() * sizeof(Utopian::LightData);
			mBuffer->MapMemory(dataOffset, dataSize, 0, (void**)&mapped);
			memcpy(mapped, lights.data(), dataSize);
			mBuffer->UnmapMemory();
		}

		virtual int GetSize()
		{
			return (NUM_MAX_LIGHTS) * sizeof(Utopian::LightData) + sizeof(constants);
		}

		struct {
			float numLights;
			glm::vec3 garbage;
		} constants;

		// Note: Todo:
		const uint32_t NUM_MAX_LIGHTS = 100;
		std::vector<Utopian::LightData> lights;
	};

	class SettingsUniformBuffer : public Utopian::Vk::ShaderBuffer
	{
	public:
		virtual void UpdateMemory()
		{
			// Map uniform buffer and update it
			uint8_t *mapped;
			mBuffer->MapMemory(0, sizeof(data), 0, (void**)&mapped);
			memcpy(mapped, &data, sizeof(data));
			mBuffer->UnmapMemory();
		}

		virtual int GetSize()
		{
			return sizeof(data);
		}

		struct {
			glm::vec3 fogColor;
			float padding;
			float fogStart;
			float fogDistance;
			int shadowsEnabled;
			int ssaoEnabled;
			int cascadeColorDebug;
		} data;
	};

	UNIFORM_BLOCK_BEGIN(CascadeBlock)
		UNIFORM_PARAM(float, cascadeSplits[4])
		UNIFORM_PARAM(glm::mat4, cascadeViewProjMat[4])
		UNIFORM_PARAM(glm::mat4, cameraViewMat)
		UNIFORM_PARAM(int, shadowSampleSize)
		UNIFORM_PARAM(int, shadowsEnabled)
	UNIFORM_BLOCK_END()
}
