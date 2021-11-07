#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 0, binding = 0) uniform sampler2D inputTexture;

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

// Reference: https://lettier.github.io/3d-game-shaders-for-beginners/dilation.html
void main()
{
   int   size         = 5;
   float separation   = 1.0f;
   float minThreshold = 0.2;
   float maxThreshold = 0.5;

   vec2 texSize   = textureSize(inputTexture, 0).xy;
   vec2 fragCoord = InTex;

   vec4 fragColor = texture(inputTexture, fragCoord);

   if (size <= 0) { return; }

   float brightestGrayscale = 0.0;
   vec4 brightestColor = fragColor;

   for (int i = -size; i <= size; ++i) {
      for (int j = -size; j <= size; ++j) {
         // For a circular shape
         if (!(distance(vec2(i, j), vec2(0, 0)) <= size)) { continue; }

         vec4 color = texture(inputTexture, (gl_FragCoord.xy + (vec2(i, j) * separation)) / texSize);
         float grayscale = dot(color.rgb, vec3(0.3, 0.59, 0.11));

         if (grayscale > brightestGrayscale) {
            brightestGrayscale = grayscale;
            brightestColor = color;
         }
      }
   }

   OutFragColor = vec4(mix(fragColor.rgb, brightestColor.rgb, smoothstep(minThreshold, maxThreshold, brightestGrayscale)), 1.0f);
   //OutFragColor = vec4(fragColor.rgb, 1.0f);
}
