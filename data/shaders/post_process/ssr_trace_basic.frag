#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 0, binding = 0) uniform sampler2D lightSampler;
layout (set = 0, binding = 1) uniform sampler2D specularSampler;
layout (set = 0, binding = 2) uniform sampler2D normalViewSampler;
layout (set = 0, binding = 3) uniform sampler2D normalWorldSampler;
layout (set = 0, binding = 4) uniform sampler2D positionSampler;

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

layout (std140, set = 1, binding = 0) uniform UBO 
{
	mat4 view;
	mat4 projection;
} ubo;

const int maxSteps = 32;
const int binarySearchSteps = 32;
const float rayStep = 1.28f;
const float rayHitThreshold = 0.09f;

const vec4 FAIL_COLOR_NOT_IN_SCREEN = vec4(0.0, 0.0, 0.0, 1.0);
const vec4 FAIL_COLOR_RAY_HIT_THRESHOLD = vec4(0.0, 0.0, 0.0, 1.0);

vec2 viewPosToNDC(vec3 positionView)
{
    // Project to NDC
    vec4 texCoord = ubo.projection * vec4(positionView, 1.0f); 
    texCoord.xy /= texCoord.w; 
    texCoord.xy = texCoord.xy * vec2(0.5f, 0.5f) + vec2(0.5f); 

    return texCoord.xy;
}

bool binarySearch(vec3 direction, vec3 positionView, out vec2 hitUV)
{
    float foundIntersection = 0.0f;
    for (int i = 0; i < binarySearchSteps; i++)
    {
        vec2 texCoord = viewPosToNDC(positionView);

        float depth = (ubo.view * vec4(texture(positionSampler, texCoord.xy).xyz, 1.0f)).z;
        float deltaDepth = positionView.z - depth;

        if (depth > positionView.z)
        {
             positionView -= direction;
        }
        else
        {
             positionView += direction;
        }

        direction *= 0.5f;
    }

    vec2 texCoord = viewPosToNDC(positionView);
    hitUV = texCoord;
    float depth = (ubo.view * vec4(texture(positionSampler, texCoord.xy).xyz, 1.0f)).z;
    float deltaDepth = positionView.z - depth;

    // Note: hit is currently not used
    bool hit = abs(deltaDepth) < rayHitThreshold ? true : false;

    if (!hit)
        OutFragColor = FAIL_COLOR_RAY_HIT_THRESHOLD;

    return hit;
}

bool rayMarch(vec3 direction, inout vec3 positionView, out vec2 hitUV)
{
    for (int i = 0; i < maxSteps; i++)
    {
        positionView += direction;

        vec2 texCoord = viewPosToNDC(positionView);

		float depth = (ubo.view * vec4(texture(positionSampler, texCoord.xy).xyz, 1.0f)).z; 
        float deltaDepth = depth - positionView.z;

        if (depth > positionView.z)
        {
            return binarySearch(direction, positionView, hitUV);
        }

        direction *= rayStep;
    }

    return false;
}

const vec3 scale = vec3(.8, .8, .8);
const float k = 19.19;
const float ssr_LLimiter = 100.1;

vec3 hash(vec3 a) {
    a = fract(a * scale);
    a += dot(a, a.yxz + k);
    return fract((a.xxy + a.yxx)*a.zyx);
}

// source: https://www.standardabweichung.de/code/javascript/webgl-glsl-fresnel-schlick-approximation
const float fresnelExp = 5.0;

float fresnel(vec3 direction, vec3 normal) {
    vec3 halfDirection = normalize(normal + direction);

    float cosine = dot(halfDirection, direction);
    float product = max(cosine, 0.0);
    float factor = 1.0 - pow(product, fresnelExp);

    return factor;
}

void main() 
{
    vec4 specular = texture(specularSampler, InTex);
    vec3 normalView = normalize(texture(normalViewSampler, InTex).xyz * 2.0f - 1.0f);
    vec3 normal = texture(normalWorldSampler, InTex).rgb;

    vec3 color = texture(lightSampler, InTex).rgb;
    vec3 positionWorld = texture(positionSampler, InTex).xyz;
    vec3 jitt = vec3(0); //hash(positionWorld);

    vec3 positionView = (ubo.view * vec4(positionWorld, 1.0f)).xyz;
    vec3 reflectionDir = normalize(reflect(normalize(positionView), normalize(normalView)));

    // Note: for testing in Sponza model
    if (specular.r < 0.4f || normal.y < 0.7f)
    {
	    OutFragColor = vec4(0);
    }
    else
    {
        vec3 hitPos = positionView;
        vec2 hitUV;
        bool hit = rayMarch(jitt + reflectionDir, hitPos, hitUV);

        vec2 coordsEdgeFact = vec2(1, 1) - pow(clamp(abs(hitUV.xy - vec2(0.5f, 0.5f)) * 2, vec2(0.0f), vec2(1.0f)), vec2(8));
        float screenEdgeFactor = clamp(min(coordsEdgeFact.x, coordsEdgeFact.y), 0, 1);

        // Note: enabling this if-statements helps with reducing artifacts below floating objects
        //if (hit)
        {
            OutFragColor = vec4(texture(lightSampler, hitUV.xy).rgb, 1.0f);
            OutFragColor *= screenEdgeFactor;
        }
        //else
        {
            //OutFragColor = vec4(0, 0, 0, 1);
        }

        // if (hitCoord != vec2(-1.0f))
        // {
        //     float L = length(hitPos - positionView);
        //     L = clamp(L * ssr_LLimiter, 0, 1);
        //     float error = 1 - L;

        //     float fresnel = fresnel(reflectionDir, normalView);

        //     OutFragColor = vec4(hitCoord.x, hitCoord.y, 0.0f, 1.0f);
        //     OutFragColor = vec4(texture(lightSampler, hitCoord).rgb * 1, 1.0f);
        // }
        // else
        // {
        //     OutFragColor = vec4(0, 0, 0, 1);
        // }
    }
}