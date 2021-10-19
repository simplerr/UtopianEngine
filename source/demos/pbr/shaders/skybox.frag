#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 0, binding = 1) uniform samplerCube samplerCubeMap;

layout (location = 0) in vec3 inUVW;

layout (location = 0) out vec4 outFragColor;

// From http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 Uncharted2Tonemap(vec3 color)
{
   float A = 0.15;
   float B = 0.50;
   float C = 0.10;
   float D = 0.20;
   float E = 0.02;
   float F = 0.30;
   float W = 11.2;
   return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

void main()
{
   vec3 color = texture(samplerCubeMap, -inUVW).rgb;

   // Tone mapping
   const float exposure = 4.5f;
   color = Uncharted2Tonemap(color * exposure);
   color = color * (1.0f / Uncharted2Tonemap(vec3(11.2f)));

   // Gamma correction
   const float gamma = 2.2f;
   color = pow(color, vec3(1.0f / gamma));

   outFragColor = vec4(color, 1.0f);
}