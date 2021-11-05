#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, set = 0, binding = 0) uniform UBO_settings
{
   int blurRadius;
} ubo;

layout (set = 1, binding = 0) uniform sampler2D inputTexture;

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

void main()
{
   int blurRange = ubo.blurRadius;
   int n = 0;
   vec2 texelSize = 1.0 / vec2(textureSize(inputTexture, 0));
   float result = 0.0;
   for (int x = -blurRange; x < blurRange; x++) 
   {
      for (int y = -blurRange; y < blurRange; y++) 
      {
         vec2 offset = vec2(float(x), float(y)) * texelSize;
         result += texture(inputTexture, InTex + offset).r;
         n++;
      }
   }

   OutFragColor = vec4(vec3(result / (float(n))), 1.0);
}