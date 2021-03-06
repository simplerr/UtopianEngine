#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 0, binding = 0) uniform sampler2D samplerSSAO;

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

void main()
{
   int blurRange = 4;
   int n = 0;
   vec2 texelSize = 1.0 / vec2(textureSize(samplerSSAO, 0));
   vec3 result = vec3(0.0);
   for (int x = -blurRange; x < blurRange; x++)
   {
      for (int y = -blurRange; y < blurRange; y++)
      {
         vec2 offset = vec2(float(x), float(y)) * texelSize;
         result += texture(samplerSSAO, InTex + offset).rgb;
         n++;
      }
   }

   OutFragColor = vec4(result / (float(n)), 1.0);
}