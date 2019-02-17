float random(vec2 st)
{
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

/**
 * From "The book of shaders" (https://thebookofshaders.com/13/)
 * Based on Morgan McGuire @morgan3d
 * https://www.shadertoy.com/view/4dS3Wd
 *
 * Other noise function that might be worth looking into:
 * http://www.kamend.com/2012/06/perlin-noise-and-glsl/
 */
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

#define OCTAVES 6
float fbm (vec2 st) {
    // Initial values
    float value = 0.0;
    float amplitude = 0.5f;

    // Loop of octaves
    for (int i = 0; i < OCTAVES; i++) {
        value += amplitude * noise(st);
        st *= 2.;
        amplitude *= .5;
    }
    return value;
}

// Calculate normal, based on https://www.gamedev.net/forums/topic/692347-finite-difference-normal-calculation-for-sphere/
// Note: This is currently not used.
vec3 getNormal(float x, float z)
{
    float offset = 1.0f;

    float hL = fbm(vec2(x - offset, z));
    float hR = fbm(vec2(x + offset, z));
    float hD = fbm(vec2(x, z - offset));
    float hU = fbm(vec2(x, z + offset));

    vec3 normal = vec3(hL - hR, 2.0f, hD - hU);
    normal = normalize(normal);
    
    return normal;
}