#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, set = 0, binding = 0) uniform UBO_settings
{
   mat4 projection;
   int dofEnabled;
   float dofStart;
   float dofRange;
} ubo;

layout (set = 1, binding = 0) uniform sampler2D normalTexture;
layout (set = 1, binding = 1) uniform sampler2D blurredTexture;
layout (set = 1, binding = 2) uniform sampler2D depthTexture;

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

// Todo: Move to common file
float eye_z_from_depth(float depth, mat4 proj)
{
   return -proj[3][2] / (proj[2][2] + depth);
}

void main()
{
   float depth = texture(depthTexture, InTex).r;
   vec3 sharp = texture(normalTexture, InTex).rgb;
   vec3 blurred = texture(blurredTexture, InTex).rgb;
   float eyeZ = -eye_z_from_depth(depth, ubo.projection);

   if (ubo.dofEnabled == 1)
   {
      float blurFar = smoothstep(ubo.dofStart, ubo.dofStart + ubo.dofRange, eyeZ);
      //float blurClose = smoothstep(5.0f, 0.0f, eyeZ);
      float blurClose = 0.0f;

      OutFragColor = vec4(mix(sharp, blurred, blurFar + blurClose), 1.0f);
   }
   else
   {
      OutFragColor = vec4(sharp, 1.0f);
   }
}
