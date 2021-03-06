#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "shared_variables.glsl"

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

layout (set = 1, binding = 0) uniform sampler2D positionSampler;
layout (set = 1, binding = 1) uniform sampler2D normalSampler;
layout (set = 1, binding = 2) uniform sampler2D albedoSampler;

const int KERNEL_SIZE = 64;

layout (std140, set = 0, binding = 0) uniform UBO_parameters
{
   vec4 kernelSamples[KERNEL_SIZE];
} ubo;

layout(std140, set = 2, binding = 0) uniform UBO_settings
{
   float radius;
   float bias;
} settings_ubo;

void main()
{
   vec3 positionWorld = texture(positionSampler, InTex).xyz;
   //vec3 normalView = texture(normalSampler, uv).xyz;
   vec3 albedo = texture(albedoSampler, InTex).rgb;
   vec3 fragPosView = (sharedVariables.viewMatrix * vec4(positionWorld, 1.0f)).xyz;
   float positionDepth = texture(positionSampler, InTex).w;

   // Get G-Buffer values
   vec3 normalView = normalize((texture(normalSampler, InTex).rgb) * 2.0 - 1.0);

   // Todo:
   // Get a random vector using a noise lookup
   //ivec2 texDim = textureSize(samplerPositionDepth, 0); 
   //ivec2 noiseDim = textureSize(ssaoNoise, 0);
   //const vec2 noiseUV = vec2(float(texDim.x)/float(noiseDim.x), float(texDim.y)/(noiseDim.y)) * inUV;  
   //vec3 randomVec = texture(ssaoNoise, noiseUV).xyz * 2.0 - 1.0;
   vec3 randomVec = vec3(1, 1, 0);
   
   // Create TBN matrix
   vec3 tangent = normalize(randomVec - normalView * dot(randomVec, normalView));
   vec3 bitangent = cross(tangent, normalView);
   mat3 TBN = mat3(tangent, bitangent, normalView);

   // Calculate occlusion value
   float occlusion = 0.0f;
   for(int i = 0; i < KERNEL_SIZE; i++)
   {
      vec3 samplePos = TBN * ubo.kernelSamples[i].xyz; 
      samplePos = fragPosView + samplePos * settings_ubo.radius; 
      
      // Project to NDC
      vec4 offset = vec4(samplePos, 1.0f);
      offset = sharedVariables.projectionMatrix * offset; 
      offset.xyz /= offset.w;
      offset.xyz = offset.xyz * 0.5f + 0.5f; 
      
      //float sampleDepth = -(texture(samplerPositionDepth, offset.xy)).w;
      float sampleDepth = (sharedVariables.viewMatrix * vec4(texture(positionSampler, offset.xy).xyz, 1.0f)).z; 

      float rangeCheck = smoothstep(0.0f, 1.0f, settings_ubo.radius / abs(fragPosView.z - sampleDepth));
      occlusion += (sampleDepth >= samplePos.z ? 1.0f : 0.0f) * rangeCheck;
      //occlusion += (sampleDepth >= samplePos.z ? 1.0f : 0.0f);
   }
   occlusion = 1.0 - (occlusion / float(KERNEL_SIZE));
   
   OutFragColor = vec4(vec3(occlusion), 1.0);
}
