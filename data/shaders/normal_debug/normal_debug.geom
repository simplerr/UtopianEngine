#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : enable

#include "shared_variables.glsl"

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

layout (location = 0) in vec3 inNormal[];

layout (location = 0) out vec3 outColor;

layout (push_constant) uniform PushConstants {
   mat4 world;
   vec4 color;

   // These exceeds the 128 byte limit
   // vec2 textureTiling;
   // vec2 pad;
} pushConstants;

void main(void)
{
   float normalLength = 0.5;
   for(int i=0; i<gl_in.length(); i++)
   {
      vec3 pos = gl_in[i].gl_Position.xyz;
      vec3 normal = normalize(inNormal[i].xyz);
      vec4 posW = pushConstants.world * vec4(pos, 1.0f);
      vec3 normalW = mat3(transpose(inverse(pushConstants.world))) * normal;
      normalW = normalize(normalW);

      mat4 viewProjection = sharedVariables.projectionMatrix * sharedVariables.viewMatrix;

      gl_Position = viewProjection * posW;
      gl_PointSize = 1;
      outColor = vec3(1.0, 0.0, 0.0);
      EmitVertex();

      gl_Position = viewProjection * vec4(posW.xyz + normalW * normalLength, 1.0f);

      outColor = vec3(0.0, 1.0, 0.0);
      EmitVertex();

      EndPrimitive();
   }
}