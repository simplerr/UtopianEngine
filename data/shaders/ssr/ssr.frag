//  Copyright (c) 2015, Ben Hopkins (kode80)
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without modification,
//  are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
//  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
//  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
//  THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
//  OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
//  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
//  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../common/sky_color.glsl"
#include "material_types.glsl"

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;
layout (location = 1) out vec4 OutRayOrigin;
layout (location = 2) out vec4 OutRayEnd;
layout (location = 3) out vec4 OutMiscDebug;

layout (set = 0, binding = 0) uniform sampler2D _MainTex;
layout (set = 0, binding = 1) uniform sampler2D _CameraDepthTexture;
layout (set = 0, binding = 2) uniform sampler2D _BackFaceDepthTex;

layout (set = 0, binding = 4) uniform sampler2D _CameraGBufferTexture1;	// R = specularity, G = material type, B = water depth, A = undefined
layout (set = 0, binding = 5) uniform sampler2D _CameraGBufferTexture2;	// World space normal (RGB), unused (A)
layout (set = 0, binding = 6) uniform sampler2D positionSampler;
layout (set = 0, binding = 7) uniform sampler2D normalSampler;

layout(std140, set = 0, binding = 8) uniform UBO_ssrSettings
{
   mat4 _CameraProjectionMatrix;             // projection matrix that maps to screen pixels (not NDC)
   mat4 _CameraInverseProjectionMatrix;      // inverse projection matrix (NDC to camera space)
   mat4 _NormalMatrix;
   mat4 _ViewMatrix;
   vec2 _RenderBufferSize;
   vec2 _OneDividedByRenderBufferSize;       // Optimization: removes 2 divisions every itteration
   float _Iterations;                        // maximum ray iterations
   float _BinarySearchIterations;            // maximum binary search refinement iterations
   float _PixelZSize;                        // Z size in camera space of a pixel in the depth buffer
   float _PixelStride;                       // number of pixels per ray step close to camera
   float _PixelStrideZCuttoff;               // ray origin Z at this distance will have a pixel stride of 1.0
   float _MaxRayDistance;                    // maximum distance of a ray
   float _ScreenEdgeFadeStart;               // distance to screen edge that ray hits will start to fade (0.0 -> 1.0)
   float _EyeFadeStart;                      // ray direction's Z that ray hits will start to fade (0.0 -> 1.0)
   float _EyeFadeEnd;                        // ray direction's Z that ray hits will be cut (0.0 -> 1.0)
   int _SSREnabled;
} ubo_settings;

#define NEAR 1.0f
#define FAR 51200.0f

// https://docs.unity3d.com/Manual/SL-UnityShaderVariables.html
const vec4 _ProjectionParams = vec4(1.0f, NEAR, FAR, 1.0f / FAR);

// Z buffer to linear 0..1 depth
float Linear01Depth(float z)
{
    // Values used to linearize the Z buffer
	// (http://www.humus.name/temp/Linearize%20depth.txt)
	// x = 1-far/near
	// y = far/near
	// z = x/far
	// w = y/far
	vec4 _ZBufferParams;
    _ZBufferParams.x = 1 - (FAR / NEAR);
    _ZBufferParams.y = (FAR / NEAR);
    _ZBufferParams.z = (_ZBufferParams.x / FAR);
    _ZBufferParams.w = (_ZBufferParams.y / FAR);

	return 1.0 / (_ZBufferParams.x * z + _ZBufferParams.y);
}

void swapIfBigger(inout float aa, inout float bb)
{
   if (aa > bb)
   {
      float tmp = aa;
      aa = bb;
      bb = tmp;
   }
}

bool rayIntersectsDepthBF(float zA, float zB, vec2 uv)
{
   float cameraZ = Linear01Depth(textureLod(_CameraDepthTexture, uv, 0).r) * -FAR;
   float backZ = Linear01Depth(textureLod(_BackFaceDepthTex, uv, 0).r) * -FAR;

   return zB <= cameraZ && zA >= backZ - ubo_settings._PixelZSize;
}

float distanceSquared(vec2 a, vec2 b) { a -= b; return dot(a, a); }

// Trace a ray in screenspace from rayOrigin (in camera space) pointing in rayDirection (in camera space)
// using jitter to offset the ray based on (jitter * _PixelStride).
//
// Returns true if the ray hits a pixel in the depth buffer
// and outputs the hitPixel (in UV space), the hitPoint (in camera space) and the number
// of iterations it took to get there.
//
// Based on Morgan McGuire & Mike Mara's GLSL implementation:
// http://casual-effects.blogspot.com/2014/08/screen-space-ray-tracing.html
//
// Implementation is based upon Kode80's Unity SSR effect
// http://www.kode80.com/blog/2015/03/11/screen-space-reflections-in-unity-5/
bool traceScreenSpaceRay(vec3 rayOrigin,
                         vec3 rayDirection,
                         float jitter,
                         out vec2 hitPixel,
                         out vec3 hitPoint,
                         out float iterationCount)
{
   // Clip to the near plane
   float rayLength = ((rayOrigin.z + rayDirection.z * ubo_settings._MaxRayDistance) > -NEAR) ?
                  (-NEAR - rayOrigin.z) / rayDirection.z : ubo_settings._MaxRayDistance;
   vec3 rayEnd = rayOrigin + rayDirection * rayLength;

   // Debug:
   OutRayOrigin = vec4(rayOrigin, 1.0f);
   OutRayEnd = vec4(rayEnd, 1.0f);

   // Project into homogeneous clip space
   vec4 H0 = ubo_settings._CameraProjectionMatrix * vec4(rayOrigin, 1.0);
   vec4 H1 = ubo_settings._CameraProjectionMatrix * vec4(rayEnd, 1.0);

   float k0 = 1.0 / H0.w;
   float k1 = 1.0 / H1.w;

   // The interpolated homogeneous version of the camera-space points
   vec3 Q0 = rayOrigin * k0;
   vec3 Q1 = rayEnd * k1;

   // Screen-space endpoints [(0-screenWidth), (0-screenHeight)]
   vec2 P0 = (H0.xy * k0 * 0.5f + 0.5f) * ubo_settings._RenderBufferSize;
   vec2 P1 = (H1.xy * k1 * 0.5f + 0.5f) * ubo_settings._RenderBufferSize;

   // If the line is degenerate, make it cover at least one pixel
   // to avoid handling zero-pixel extent as a special case later
   P1 += (distanceSquared(P0, P1) < 0.0001) ? 0.01 : 0.0;

   vec2 delta = P1 - P0;

   // Permute so that the primary iteration is in x to collapse
   // all quadrant-specific DDA cases later
   bool permute = false;
   if (abs(delta.x) < abs(delta.y)) {
      // This is a more-vertical line
      permute = true; delta = delta.yx; P0 = P0.yx; P1 = P1.yx;
   }

   float stepDir = sign(delta.x);
   float invdx = stepDir / delta.x;

   // Track the derivatives of Q and k
   vec3  dQ = (Q1 - Q0) * invdx;
   float dk = (k1 - k0) * invdx;
   vec2  dP = vec2(stepDir, delta.y * invdx);

   // Calculate pixel stride based on distance of ray origin from camera.
   // Since perspective means distant objects will be smaller in screen space
   // we can use this to have higher quality reflections for far away objects
   // while still using a large pixel stride for near objects (and increase performance)
   // this also helps mitigate artifacts on distant reflections when we use a large
   // pixel stride.
   float strideScaler = 1.0 - min(1.0, -rayOrigin.z / ubo_settings._PixelStrideZCuttoff);
   float pixelStride = 1.0 + strideScaler * ubo_settings._PixelStride;

   // Scale derivatives by the desired pixel stride and then
   // offset the starting values by the jitter fraction
   dP *= pixelStride; dQ *= pixelStride; dk *= pixelStride;
   P0 += dP * jitter; Q0 += dQ * jitter; k0 += dk * jitter;

   float i, zA = 0.0, zB = 0.0;

   // Track ray step and derivatives in a vec4 to parallelize
   vec4 pqk = vec4(P0, Q0.z, k0);
   vec4 dPQK = vec4(dP, dQ.z, dk);
   bool intersect = false;

   for(i=0; i<ubo_settings._Iterations && intersect == false; i++)
   {
      pqk += dPQK;

      zA = zB;
      zB = (dPQK.z * 0.5 + pqk.z) / (dPQK.w * 0.5 + pqk.w);
      swapIfBigger(zB, zA);

      hitPixel = permute ? pqk.yx : pqk.xy;
      hitPixel *= ubo_settings._OneDividedByRenderBufferSize;

      intersect = rayIntersectsDepthBF(zA, zB, hitPixel);

      OutMiscDebug = vec4(hitPixel, dPQK.xy);
   }

   // Binary search refinement
   if (pixelStride > 1.0 && intersect)
   {
      pqk -= dPQK;
      dPQK /= pixelStride;

      float originalStride = pixelStride * 0.5;
      float stride = originalStride;

      zA = pqk.z / pqk.w;
      zB = zA;

      for(float j=0; j<ubo_settings._BinarySearchIterations; j++)
      {
         pqk += dPQK * stride;

         zA = zB;
         zB = (dPQK.z * -0.5 + pqk.z) / (dPQK.w * -0.5 + pqk.w);
         swapIfBigger(zB, zA);

         hitPixel = permute ? pqk.yx : pqk.xy;
         hitPixel *= ubo_settings._OneDividedByRenderBufferSize;

         originalStride *= 0.5;
         stride = rayIntersectsDepthBF(zA, zB, hitPixel) ? -originalStride : originalStride;
      }
   }

   Q0.xy += dQ.xy * i;
   Q0.z = pqk.z;
   hitPoint = Q0 / pqk.w;
   iterationCount = i;

   return intersect;
}

float calculateAlphaForIntersection(bool intersect,
                                    float iterationCount,
                                    float specularStrength,
                                    vec2 hitPixel,
                                    vec3 hitPoint,
                                    vec3 vsRayOrigin,
                                    vec3 vsRayDirection)
{
   float alpha = min(1.0, specularStrength * 1.0);

   // Fade ray hits that approach the maximum iterations
   //alpha *= 1.0 - max(0.15, (iterationCount / ubo_settings._Iterations));
   // Note: Hack to reduce the fade distance
   const float FADE_MARGIN = 400;
   alpha *= (1.0 - ((iterationCount - FADE_MARGIN) / (ubo_settings._Iterations - FADE_MARGIN)));
   alpha = min(1, alpha);

   // Fade ray hits that approach the screen edge
   float screenFade = ubo_settings._ScreenEdgeFadeStart;
   vec2 hitPixelNDC = (hitPixel * 2.0 - 1.0);
   float maxDimension = min(1.0, max(abs(hitPixelNDC.x), abs(hitPixelNDC.y)));
   alpha *= 1.0 - (max(0.0, maxDimension - screenFade) / (1.0 - screenFade));

   // Fade ray hits base on how much they face the camera
   float eyeFadeStart = ubo_settings._EyeFadeStart;
   float eyeFadeEnd = ubo_settings._EyeFadeEnd;
   swapIfBigger(eyeFadeStart, eyeFadeEnd);

   float eyeDirection = clamp(vsRayDirection.z, eyeFadeStart, eyeFadeEnd);
   alpha *= 1.0 - ((eyeDirection - eyeFadeStart) / (eyeFadeEnd - eyeFadeStart));

   // Fade ray hits based on distance from ray origin
   alpha *= 1.0 - clamp(distance(vsRayOrigin, hitPoint) / ubo_settings._MaxRayDistance, 0.0, 1.0);

   alpha *= int(intersect);

   return alpha;
}

void main()
{
   vec3 worldNormal = texture(normalSampler, InTex).rgb;
   vec4 specular = texture(_CameraGBufferTexture1, InTex);
   float reflectiveness = specular.r;

   // Only reflect ground planes for now
   if (reflectiveness == 0.0f || worldNormal.y < 0.99f)
      discard;

   float decodedDepth = Linear01Depth(texture(_CameraDepthTexture, InTex).r);

   vec3 worldPosition = texture(positionSampler, InTex).xyz;
   vec3 viewPosition = (ubo_settings._ViewMatrix * vec4(worldPosition, 1.0f)).xyz;

   // Note: Already in view space
   // Transfering the normal to view space at this stage should remove the wobble effect when moving the camera
   vec3 decodedNormal = (texture(_CameraGBufferTexture2, InTex)).rgb * 2.0 - 1.0;
   //mat3 normalMatrix = transpose(inverse(mat3(ubo_settings._ViewMatrix)));
   //decodedNormal = mat3(ubo_settings._NormalMatrix) * decodedNormal;
   //decodedNormal = normalMatrix * decodedNormal;

   vec3 vsRayOrigin = viewPosition;
   vec3 vsRayDirection = normalize(reflect(normalize(vsRayOrigin), normalize(decodedNormal)));

   vec2 hitPixel;
   vec3 hitPoint;
   float iterationCount;

   vec2 uv2 = InTex * ubo_settings._RenderBufferSize;
   float c = (uv2.x + uv2.y) * 0.25;
   float jitter = mod(c, 1.0);

   vec4 reflectionColor = vec4(0.0f);
   float alpha = 0.0f;
   if (ubo_settings._SSREnabled == 1)
   {
      bool intersect = traceScreenSpaceRay(vsRayOrigin, vsRayDirection, jitter, hitPixel, hitPoint, iterationCount);
      alpha = calculateAlphaForIntersection(intersect, iterationCount, reflectiveness, hitPixel, hitPoint, vsRayOrigin, vsRayDirection);
      reflectionColor = vec4((texture(_MainTex, hitPixel)).rgb, alpha);
   }

   vec4 fallbackColor = vec4(0.0f);
   uint materialType = uint(specular.g);
   if (materialType == MATERIAL_TYPE_WATER)
   {
      // Sky sphere color
      vec3 toEyeW = normalize(ubo_parameters.eyePos + worldPosition); // Todo: Note: the + sign is due to the fragment world position is negated for some reason
      vec3 reflection = reflect(toEyeW, worldNormal);
      reflection.y *= -1; // Note: -1
      SkyOutput skyColor = GetSkyColor(reflection);
      fallbackColor = skyColor.skyColor;
   }

   reflectionColor = mix(fallbackColor, reflectionColor, alpha);

   OutFragColor = reflectionColor;
}