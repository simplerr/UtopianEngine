#version 450

layout (location = 0) in vec3 InPosW;
layout (location = 1) in vec3 InNormalW;
layout (location = 2) in vec3 InEyePosW;
layout (location = 3) in vec3 InColor;
layout (location = 4) in vec2 InTex;
layout (location = 5) in vec4 InTangentL;

layout (location = 0) out vec4 OutColor;

layout (set = 1, binding = 0) uniform sampler2D diffuseSampler;
layout (set = 1, binding = 1) uniform sampler2D normalSampler;

void main()
{
   vec4 diffuse = texture(diffuseSampler, InTex);
   vec4 normal = texture(normalSampler, InTex);

   if (diffuse.a < 0.5f)
      discard;

   vec3 N = normalize(InNormalW);

   // No tangent -> no normal mapping
   if (InTangentL != vec4(0.0f))
   {
      vec3 T = normalize(InTangentL.xyz);
      vec3 B = cross(InNormalW, InTangentL.xyz) * InTangentL.w;
      mat3 TBN = mat3(T, B, N);
      N = TBN * normalize(normal.xyz * 2.0 - vec3(1.0));
   }

   vec3 lightDir = vec3(0.5, 0.5, -0.5);
   float diffuseFactor = max(dot(lightDir, N), 0.0) + 0.2;
   OutColor = vec4(diffuse.rgb * diffuseFactor, 1.0f);
   //OutColor = vec4(N, 1.0f);
}
