#pragma once

#include <glm/glm.hpp>
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/VertexDescription.h"

namespace Utopian::Vk
{
   /**
    * The default vertex layout used by the renderer.
    *
    * Matches attribute layout in data\shaders\include\vertex.glsl.
    */
   struct Vertex
   {
      Vertex() {
         pos = glm::vec3(0.0f);
         normal = glm::vec3(0.0f);
         uv = glm::vec2(0.0f);
         color = glm::vec3(1.0f);
         tangent = glm::vec4(0.0f);
         jointIndices = glm::vec4(0.0f);
         jointWeights = glm::vec4(0.0f);
      }

      Vertex(glm::vec3 pos) : pos(pos) {}

      Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 uv,
             glm::vec3 tangent = glm::vec3(0.0f), glm::vec3 color = glm::vec3(1.0f))
         : pos(position), normal(normal), uv(uv), tangent(tangent, 1.0f), color(color) {}

      static VertexDescription GetDescription()
      {
         VertexDescription description;
         description.AddBinding(BINDING_0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);

         // We need to tell Vulkan about the memory layout for each attribute
         // 5 attributes: position, normal, texture coordinates, tangent and color
         // See Vertex struct
         description.AddAttribute(BINDING_0, Vec3Attribute()); // Location 0 : InPosL
         description.AddAttribute(BINDING_0, Vec3Attribute()); // Location 1 : InNormalL
         description.AddAttribute(BINDING_0, Vec2Attribute()); // Location 2 : InTex
         description.AddAttribute(BINDING_0, Vec3Attribute()); // Location 3 : InColor
         description.AddAttribute(BINDING_0, Vec4Attribute()); // Location 4 : InTangentL
         description.AddAttribute(BINDING_0, Vec4Attribute()); // Location 5 : InJointIndices
         description.AddAttribute(BINDING_0, Vec4Attribute()); // Location 6 : InJointWeights
         return description;
      }

      glm::vec3 pos;
      glm::vec3 normal;
      glm::vec2 uv;
      glm::vec3 color;
      glm::vec4 tangent;
      glm::vec4 jointIndices;
      glm::vec4 jointWeights;
   };

   struct ScreenQuadVertex
   {
      static VertexDescription GetDescription()
      {
         VertexDescription description;
         description.AddBinding(BINDING_0, sizeof(ScreenQuadVertex), VK_VERTEX_INPUT_RATE_VERTEX);
         description.AddAttribute(BINDING_0, Vec3Attribute());   // InPosL
         description.AddAttribute(BINDING_0, Vec2Attribute());   // InTex
         return description;
      }

      glm::vec3 pos;
      glm::vec2 uv;
   };

   struct TerrainVertex
   {
      static VertexDescription GetDescription()
      {
         VertexDescription description;
         description.AddBinding(BINDING_0, sizeof(TerrainVertex), VK_VERTEX_INPUT_RATE_VERTEX);
         description.AddAttribute(BINDING_0, Vec4Attribute());   // InPosL
         description.AddAttribute(BINDING_0, Vec4Attribute());   // InNormal
         return description;
      }

      glm::vec4 position;
      glm::vec4 normal;
   };
}
