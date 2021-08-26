
/**
 * The default vertex layout used by the renderer.
 *
 * Matches attribute layout in source\utopian\vulkan\Vertex.h.
 */
layout (location = 0) in vec3 InPosL;
layout (location = 1) in vec3 InNormalL;
layout (location = 2) in vec2 InTex;
layout (location = 3) in vec3 InColor;
layout (location = 4) in vec4 InTangentL;
layout (location = 5) in vec4 InJointIndices;
layout (location = 6) in vec4 InJointWeights;