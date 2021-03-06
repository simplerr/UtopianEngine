//! Corresponds to the C++ class Material. Stores the ambient, diffuse and specular colors for a material.
struct Material
{
   vec4 ambient;
   vec4 diffuse;
   vec4 specular;
};

struct Light
{
   // Color
   Material material;

   vec3 pos;
   float range;

   vec3 dir;
   float spot;

   vec3 att;
   float type;

   vec3 intensity;
   float id;

   // Note: this padding corresponds to the padding in Vulkan::Material
   vec4 pad;
};