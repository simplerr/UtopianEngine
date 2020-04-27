#include "gerstner.glsl"

layout (std140, set = 0, binding = 6) uniform UBO_viewProjection 
{
	mat4 projection;
	mat4 view;
    vec4 frustumPlanes[6];
    vec3 eyePos;
    float time;
} ubo_camera;

layout (std140, set = 0, binding = 7) uniform UBO_settings
{
    vec2 viewportSize;
    float edgeSize; // The size in pixels that all edges should have
	float tessellationFactor;
    float amplitude;
    float textureScaling;
    float bumpmapAmplitude;
    int wireframe;
} ubo_settings;

 
vec3 calculateWavePosition(vec2 pos, float time, inout vec3 normal)
{
    float timeFactor = 0.002;
	vec3 wavePosition = gerstner_wave(pos, time * timeFactor, normal).xyz;
    
    return wavePosition;
}
