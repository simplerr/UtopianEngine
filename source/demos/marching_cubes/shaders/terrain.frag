#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosW;
layout (location = 1) in vec3 InNormalW;
layout (location = 2) in vec3 InEyePosW;
layout (location = 3) in vec3 InColor;

layout (location = 0) out vec4 OutFragColor;
layout (location = 1) out vec4 OutPosition;

const vec3 lightDir = vec3(0.5, 0.5, 0.5);

const vec3 peakColor = vec3(1.0f);
const vec3 grassColor = vec3(19.0 / 255, 149.0 / 255, 21.0 / 255);
const vec3 dirtColor = vec3(155.0 / 255, 118.0 / 255, 83.0 / 255);
const vec3 rockColor = vec3(0.33f);

layout (std140, set = 0, binding = 1) uniform UBO_fragInput
{
   vec3 brushPos;
   int mode; // 0 = phong, 1 = normals, 2 = block cells
} ubo_input;

void main(void)
{
   vec3 tmp = InEyePosW;
   tmp = InColor;

   vec3 originPos = InPosW - 796 * 32 * 10.0f;//vec3(256000.0f);

   vec3 lightVec = normalize(lightDir);
   float diffuseFactor = max(dot(lightVec, InNormalW), 0.0f);

   vec3 finalColor = grassColor;

   vec3 lowAltitude = mix(rockColor, grassColor, clamp((originPos.y - 500.0) / 50.0, 0.0, 1.0));
   lowAltitude = mix(lowAltitude, dirtColor, clamp((originPos.y - 800.0) / 100.0, 0.0, 1.0));
   finalColor = mix(lowAltitude, peakColor, clamp((originPos.y - 1000.0) / 100.0, 0.0, 1.0));

   finalColor = mix(finalColor, rockColor, clamp((1.0 - InNormalW.y - 0.4) / 0.4, 0.0, 1.0));

   vec3 shadedColor = finalColor * (0.1 + vec3(diffuseFactor) * 1.0);

   // Fog
   float distToEye = distance(InEyePosW, InPosW);
   float fogLerp = clamp((distToEye - 1200.0) / 5000.0, 0.0, 1.0);
   shadedColor = mix(shadedColor, vec3(0.75), fogLerp);

   // Red marker close to the brush position
   shadedColor = mix(vec3(1.0, 0.0, 0.0), shadedColor, clamp((distance(ubo_input.brushPos, InPosW) - 50) / 100.0, 0.6, 1.0));

   if (ubo_input.mode == 0)
      OutFragColor = vec4(shadedColor, 1.0f);
   else if (ubo_input.mode == 1)
      OutFragColor = vec4(InNormalW, 1.0f);
   else
      OutFragColor *= vec4(InColor, 1.0f);

   OutPosition = vec4(InPosW, 1.0f);
}