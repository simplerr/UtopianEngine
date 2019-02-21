#pragma once
#include <glm/glm.hpp>

namespace Utopian
{
	struct RenderingSettings
	{
		glm::vec4 fogColor;
		bool deferredPipeline;
		float fogStart;
		float fogDistance;
		float ssaoRadius = 6.0f;
		float ssaoBias = 0.0f;
		int blurRadius = 2;
		float grassViewDistance = 0 * 1800.0f;
		int blockViewDistance = 2;
		bool shadowsEnabled = true;
		bool normalMapping = true;
		bool ssaoEnabled = true;
		bool godRaysEnabled = true;
		int shadowSampleSize = 1;
		bool cascadeColorDebug = 0;
		float cascadeSplitLambda = 0.927f;
		float nearPlane = 1.0f;
		float farPlane = 25600.0f;
		float sunSpeed = 1.0f;
		float sunInclination = 45.0f;
		float sunAzimuth = 0.0f;
		float tessellationFactor = 2.8f;
		float terrainAmplitude = 3000.0f;
		float terrainTextureScaling = 2.2f;
		bool terrainWireframe = 0;
	};
}
