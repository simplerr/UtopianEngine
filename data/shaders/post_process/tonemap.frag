#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, set = 0, binding = 0) uniform UBO_settings
{
   int tonemapping; // 0 = Reinhard, 1 = Uncharted 2, 2 = Exposure
   float exposure;
} settings_ubo;

layout (set = 0, binding = 1) uniform sampler2D hdrSampler;
layout (set = 0, binding = 2) uniform sampler2D bloomSampler;

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutColor;

vec3 ReinhardTonemap(vec3 hdrColor)
{
   return hdrColor / (hdrColor + vec3(1.0));
}

// From http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 Uncharted2Tonemap(vec3 hdrColor)
{
   float A = 0.15;
   float B = 0.50;
   float C = 0.10;
   float D = 0.20;
   float E = 0.02;
   float F = 0.30;
   float W = 11.2;
   vec3 x = hdrColor;

   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 ExposureTonemap(vec3 hdrColor)
{
   return vec3(1.0) - exp(-hdrColor * settings_ubo.exposure);
}

// From https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 ACESFilm(vec3 x)
{
   float a = 2.51f;
   float b = 0.03f;
   float c = 2.43f;
   float d = 0.59f;
   float e = 0.14f;
   return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0f, 1.0f);
}

void main()
{
   vec3 hdrColor = texture(hdrSampler, InTex).rgb;
   vec3 bloomColor = texture(bloomSampler, InTex).rgb;

   hdrColor += bloomColor;
  
   vec3 mapped = vec3(0.0f);
   if (settings_ubo.tonemapping == 0)
      mapped = ReinhardTonemap(hdrColor);
   else if (settings_ubo.tonemapping == 1)
      mapped = Uncharted2Tonemap(hdrColor);
   else if (settings_ubo.tonemapping == 2)
      mapped = ExposureTonemap(hdrColor);
   else if (settings_ubo.tonemapping == 3)
      mapped = hdrColor;
   else if (settings_ubo.tonemapping == 4)
      mapped = ACESFilm(hdrColor);

   // Gamma correction
   // Note: This makes the image gray and washed out
   // if (settings_ubo.tonemapping != 3)
   // {
   //    const float gamma = 2.2;
   //    mapped = pow(mapped, vec3(1.0 / gamma));
   // }

   OutColor = vec4(mapped, 1.0);
}