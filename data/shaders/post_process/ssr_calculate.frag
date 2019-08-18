#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 0, binding = 0) uniform sampler2D lightSampler;
layout (set = 0, binding = 1) uniform sampler2D specularSampler;
layout (set = 0, binding = 2) uniform sampler2D normalSampler;
layout (set = 0, binding = 3) uniform sampler2D positionSampler;

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

layout (std140, set = 1, binding = 0) uniform UBO 
{
	mat4 view;
	mat4 projection;
} ubo;

const int maxSteps = 32;
const int binarySearchSteps = 32;
const float rayStep = 1.12f;

vec2 viewPosToNDC(vec3 positionView)
{
    // Project to NDC
    vec4 texCoord = ubo.projection * vec4(positionView, 1.0f); 
    texCoord.xy /= texCoord.w; 
    texCoord.xy = texCoord.xy * vec2(0.5f, 0.5f) + vec2(0.5f); 

    return texCoord.xy;
}

vec3 binarySearch(vec3 direction, vec3 positionView)
{
    float foundIntersection = 0.0f;
    for (int i = 0; i < binarySearchSteps; i++)
    {
        vec2 texCoord = viewPosToNDC(positionView);

        float depth = (ubo.view * vec4(texture(positionSampler, texCoord.xy).xyz, 1.0f)).z;
        float deltaDepth = positionView.z - depth;

        if (direction.z < 0.0f && deltaDepth < 0.0f)
             positionView -= direction;
        else
        {
            positionView += direction;
            foundIntersection = 1.0f;
        }

        direction *= 0.5f;
    }

    vec2 texCoord = viewPosToNDC(positionView);

    return vec3(texCoord.xy, foundIntersection);
}

vec2 rayMarch(vec3 direction, inout vec3 positionView)
{
    for (int i = 0; i < maxSteps; i++)
    {
        positionView += direction;

        vec2 texCoord = viewPosToNDC(positionView);

		float depth = (ubo.view * vec4(texture(positionSampler, texCoord.xy).xyz, 1.0f)).z; 
        float deltaDepth = positionView.z - depth;

        //if (direction.z < 0.0f && deltaDepth <= 0.0f)
        //if (abs(direction.z) - abs(deltaDepth) >= 5000.2)
        if (depth > positionView.z)
        {
            if (deltaDepth > -150.3f)
            {
                //return texCoord.xy;
                vec3 binaryResult = binarySearch(direction, positionView);
                if (binaryResult.z != 0.0f)
                    return binaryResult.xy;
                else
                    return vec2(0.0f, 0.0f);
            }
        }

        direction *= rayStep;
    }

    return vec2(0.0f, 0.0f);
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
    vec3 normalView = normalize(texture(normalSampler, InTex).xyz * 2.0f - 1.0f);

    vec3 color = texture(lightSampler, InTex).rgb;
    vec3 positionWorld = texture(positionSampler, InTex).xyz;
    vec3 jitt = vec3(0); //hash(positionWorld);

    vec3 positionView = (ubo.view * vec4(positionWorld, 1.0f)).xyz;
    vec3 reflectionDir = normalize(reflect(normalize(positionView), normalize(normalView)));

    if (specular.r == 0.0f)
    {
	    OutFragColor = vec4(0);
        //return;
    }
    else
    {
        vec3 hitPos = positionView;
        vec2 hitCoord = rayMarch(jitt + reflectionDir * 10, hitPos);

        float L = length(hitPos - positionView);
        L = clamp(L * ssr_LLimiter, 0, 1);
        float error = 1 - L;

        float fresnel = fresnel(reflectionDir, normalView);

        OutFragColor = vec4(hitCoord.x, hitCoord.y, 0.0f, 1.0f);
        OutFragColor = vec4(texture(lightSampler, hitCoord).rgb * 1, 1.0f);
    }

    //OutFragColor = vec4(vec3(color), 1.0f);
    //OutFragColor = vec4((ubo.view * vec4(texture(positionSampler, InTex.xy).xyz, 1.0f)).xyz, 1.0f); 

    // OutFragColor = vec4(vec3(normalView), 1.0f);
    // reflectionDir.z *= -1;
    // OutFragColor = vec4(vec3(reflectionDir), 1.0f);
    //OutFragColor = vec4(texture(lightSampler, InTex).rgb, 1.0f);

    //OutFragColor = vec4(vec3(positionView.z / -10000.0f), 1.0f); 
}
