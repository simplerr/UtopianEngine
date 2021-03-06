#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, set = 0, binding = 0) uniform UBO_parameters
{
   float radialBlurScale;
   float radialBlurStrength;
   vec2 radialOrigin;
} ubo_parameters;

layout (set = 1, binding = 0) uniform sampler2D sunSampler;

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

void main()
{
   ivec2 texDim = textureSize(sunSampler, 0);
   vec2 radialSize = vec2(1.0 / texDim.s, 1.0 / texDim.t);

   vec2 uv = InTex;

   // float radialBlurScale = ubo_parameters.radialBlurScale;
   // float radialBlurStrength = ubo_parameters.radialBlurStrength;
   // vec2 radialOrigin = ubo_parameters.radialOrigin;

   float radialBlurScale = 0.85;
   float radialBlurStrength = 0.95;
   vec2 radialOrigin = vec2(0.5, 0.5);

   vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
   uv += radialSize * 0.5 - ubo_parameters.radialOrigin;

#define samples 64

   for (int i = 0; i < samples; i++)
   {
      float scale = 1.0 - radialBlurScale * (float(i) / float(samples-1));
      color += texture(sunSampler, uv * scale + ubo_parameters.radialOrigin);
   }

   OutFragColor = (color / samples) * radialBlurStrength;
}
