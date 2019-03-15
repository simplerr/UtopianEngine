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
		bool fxaaEnabled = true;
		bool fxaaDebug = false;
		bool godRaysEnabled = true;
		float fxaaThreshold = 0.125;
		int shadowSampleSize = 1;
		bool cascadeColorDebug = 0;
		float cascadeSplitLambda = 0.927f;
		float sunSpeed = 0.0f;
		float sunInclination = 45.0f;
		float sunAzimuth = 0.0f;
		float tessellationFactor = 2.8f;
		float terrainTextureScaling = 75.0;
		float terrainBumpmapAmplitude = 14.0;
		bool terrainWireframe = 0;
		float exposure = 1.4f;
	};
}
