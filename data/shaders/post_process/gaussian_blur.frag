#version 450

layout (set = 0, binding = 1) uniform sampler2D hdrSampler;

layout (set = 0, binding = 0) uniform UBO_settings
{
   float blurScale;
   float blurStrength;
} ubo;

layout (push_constant) uniform PushConstants {
   int blurDirection;
} pushConstants;

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

void main()
{
   float weight[5];
   weight[0] = 0.227027;
   weight[1] = 0.1945946;
   weight[2] = 0.1216216;
   weight[3] = 0.054054;
   weight[4] = 0.016216;

   vec2 tex_offset = 1.0 / textureSize(hdrSampler, 0) * ubo.blurScale;
   vec3 result = texture(hdrSampler, InTex).rgb * weight[0];
   for(int i = 1; i < 5; ++i)
   {
      if (pushConstants.blurDirection == 0)
      {
         // Horizontal
         result += max(vec3(0.0f), texture(hdrSampler, InTex + vec2(tex_offset.x * i, 0.0)).rgb * weight[i] * ubo.blurStrength);
         result += max(vec3(0.0f), texture(hdrSampler, InTex - vec2(tex_offset.x * i, 0.0)).rgb * weight[i] * ubo.blurStrength);
      }
      else
      {
         // Vertical
         result += max(vec3(0.0f), texture(hdrSampler, InTex + vec2(0.0, tex_offset.y * i)).rgb * weight[i] * ubo.blurStrength);
         result += max(vec3(0.0f), texture(hdrSampler, InTex - vec2(0.0, tex_offset.y * i)).rgb * weight[i] * ubo.blurStrength);
      }
   }

   OutFragColor = vec4(result, 1.0);
}