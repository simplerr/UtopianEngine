#pragma once

#include <glm/glm.hpp>

namespace Utopian
{
   enum LightType
   {
      DIRECTIONAL_LIGHT,
      POINT_LIGHT,
      SPOT_LIGHT
   };

   struct Material
   {
      Material() {
         //ZeroMemory(this, sizeof(this));
      }

      Material(glm::vec4 color) {
         ambient = diffuse = specular = color;
      }

      Material(glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular) {
         this->ambient = ambient;
         this->diffuse = diffuse;
         this->specular = specular;
      }

      // https://www.khronos.org/registry/OpenGL/specs/gl/glspec45.core.pdf#page=159
      glm::vec4 padding; // Note: This padding corresponds to the padding in glsl Light struct
      glm::vec4 ambient;
      glm::vec4 diffuse;
      glm::vec4 specular;  // w = SpecPower
   };


   /* This matches the struct in the shader. */
   struct LightData
   {
   public:
      // Light color
      Material material;

      // Packed into 4D vector: (position, range)
      glm::vec3 position;
      float range;

      // Packed into 4D vector: (direction, spot)
      glm::vec3 direction;
      float spot;

      // Packed into 4D vector: (att, type)
      glm::vec3 att;
      float type; // 0 = directional, 1 = point light, 2 = spot light

      // Light intensity
      glm::vec3 intensity;
      float padding;
   };
}