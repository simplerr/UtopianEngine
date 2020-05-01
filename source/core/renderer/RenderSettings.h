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
		bool ssaoEnabled = false;
		bool ssrEnabled = false;
		bool skyboxReflections = true;
		bool waterEnabled = true;
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
		float terrainTextureScaling = 200.0;
		float terrainBumpmapAmplitude = 10.0;
		bool terrainWireframe = 0;
		int tonemapping = 3; // 0 = Reinhard, 1 = Uncharted 2, 2 = Exposure, 3 = None
		float exposure = 1.4f;
		float bloomThreshold = 1.5f;
		float windStrength = 5.0f;
		float windFrequency = 10000.0f;
		bool windEnabled = true;
		// Water
		float waterLevel = 0.0f;
		glm::vec3 waterColor = glm::vec3(0.0f, 0.1f, 0.4f);
		glm::vec3 foamColor = glm::vec3(0.9f);
		float waveSpeed = 1.0f;
		float foamSpeed = 4.0f;
		float waterDistortionStrength = 0.02f;
		float shorelineDepth = 150.0f;
		float waveFrequency = 90.0f;
		float waterSpecularity = 128.0f;
	};
}
