//! Corresponds to the C++ class LightColor. Stores the ambient, diffuse and specular colors for a material.
struct PhongMaterial
{
   vec4 ambient;
   vec4 diffuse;
   vec4 specular;
};

struct Light
{
   // Color
   vec4 color;

   vec3 pos;
   float range;

   vec3 dir;
   float spot;

   vec3 att;
   float type;

   vec3 intensity;
   float id;

   // Note: this padding corresponds to the padding in Vulkan::LightColor
   vec4 pad;
};