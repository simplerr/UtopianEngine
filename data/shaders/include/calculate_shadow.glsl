layout (set = 1, binding = 4) uniform sampler2DArray shadowSampler;

layout (std140, set = 0, binding = 4) uniform UBO_cascades
{
	vec4 cascadeSplits;
	mat4 cascadeViewProjMat[4];
	mat4 cameraViewMat;
	int shadowSampleSize;
	int shadowsEnabled;
} cascades_ubo;

#define SHADOW_MAP_CASCADE_COUNT 4

float calculateShadow(vec3 position, vec3 normal, vec3 lightDir, out uint cascadeIndex)
{
    if (cascades_ubo.shadowsEnabled == 0)
        return 1.0f;

	// Get cascade index for the current fragment's view position
	vec3 viewPosition = (cascades_ubo.cameraViewMat * vec4(position, 1.0f)).xyz;
	cascadeIndex = 0;
	for(uint i = 0; i < SHADOW_MAP_CASCADE_COUNT - 1; ++i) {
		if(viewPosition.z < cascades_ubo.cascadeSplits[i]) {	
			cascadeIndex = i + 1;
		}
	}

	vec4 lightSpacePosition = cascades_ubo.cascadeViewProjMat[cascadeIndex] * vec4(position, 1.0f);
	vec4 projCoordinate = lightSpacePosition / lightSpacePosition.w; // Perspective divide 
	projCoordinate.xy = projCoordinate.xy * 0.5f + 0.5f;

	float shadow = 0.0f;
	vec2 texelSize = 1.0 / textureSize(shadowSampler, 0).xy;
	int count = 0;
	int range = cascades_ubo.shadowSampleSize;
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			// If fragment depth is outside frustum do no shadowing
			if (projCoordinate.z <= 1.0f && projCoordinate.z > -1.0f)
			{
				vec2 offset = vec2(x, y) * texelSize;
				float closestDepth = texture(shadowSampler, vec3(projCoordinate.xy + offset, cascadeIndex)).r;
				float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.00000065); 
				bias = 0.0005; // This seems to fix shadow acne for now
				shadow += ((projCoordinate.z - bias) > closestDepth ? 0.0f : 1.0f);
			}
			else
			{
				shadow += 1.0f;
			}

			count++;
		}
	}

	shadow /= (count);

	return shadow;
}
