#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosW;
layout (location = 1) in vec3 InNormalW;
layout (location = 2) in vec3 InEyePosW;
layout (location = 3) in vec3 InColor;

layout (location = 0) out vec4 OutFragColor;

const vec3 lightDir = vec3(0.5, 0.5, 0.5);

layout (std140, set = 0, binding = 1) uniform UBO_settings
{
   int mode; // 0 = phong, 1 = normals, 2 = block cells
} ubo_settings;

void main(void)
{
   vec3 tmp = InPosW;
   tmp = InEyePosW;
   tmp = InColor;

   vec3 lightVec = normalize(lightDir);
   float diffuseFactor = max(dot(lightVec, InNormalW), 0.0f);

   vec3 groundColor = vec3(19.0 / 255, 109.0 / 255, 21.0 / 255);
   vec3 shadedColor = groundColor * (0.1 + vec3(diffuseFactor) * 1.0);

   if (ubo_settings.mode == 0)
      OutFragColor = vec4(shadedColor, 1.0f);
   else if (ubo_settings.mode == 1)
      OutFragColor = vec4(InNormalW, 1.0f);
   else
      OutFragColor *= vec4(InColor, 1.0f);
}