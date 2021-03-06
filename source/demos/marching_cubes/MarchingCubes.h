#pragma once

#include <string>
#include <glm/glm.hpp>
#include <vulkan/RenderTarget.h>
#include <vulkan/handles/Semaphore.h>
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/ShaderBuffer.h"
#include "utility/Common.h"

using namespace Utopian;

class MiniCamera;
class Block;

struct BlockKey
{
   BlockKey(int32_t _x, int32_t _y, int32_t _z)
      : x(_x), y(_y), z(_z) {

   }

   int32_t x, y, z;
};

bool operator<(BlockKey const& a, BlockKey const& b);

/**
 * References:
 * http://paulbourke.net/geometry/polygonise/
 * http://http.developer.nvidia.com/GPUGems3/gpugems3_ch01.html
 * http://www.icare3d.org/codes-and-projects/codes/opengl_geometry_shader_marching_cubes.html
 * https://0fps.net/2012/07/12/smooth-voxel-terrain-part-2/
 */
class MarchingCubes
{
public:
   MarchingCubes(Utopian::Window* window);
   ~MarchingCubes();

   void Run();

   void DestroyCallback();
   void UpdateCallback();
   void DrawCallback();

   /** Adds the blocks within the viewing distance range. */
   void UpdateBlockList();
   void UpdateUi();

   void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
   void InitResources();
   void InitMarchingCubesJob(Vk::Device* device, uint32_t width, uint32_t height);
   void InitTerrainJob(Vk::Device* device, uint32_t width, uint32_t height);
   void InitNoiseJob(Vk::Device* device);
   void InitBrushJob(Vk::Device* device);
   void InitIntersectionJob(Vk::Device* device, uint32_t width, uint32_t height);

   void RunMarchingCubesJob();
   void RunTerrainJob();
   void RunNoiseJob();
   void RunBrushJob();
   void RunIntersectionJob();

   void ActivateBlockRegeneration();
   glm::ivec3 GetBlockCoordinate(glm::vec3 position);

private:

   std::map<BlockKey, Block*> mBlockList;
   SharedPtr<MiniCamera> mCamera;
   Vk::VulkanApp* mVulkanApp;
   Utopian::Window* mWindow;
   const glm::vec3 mOrigin = glm::vec3(256000.0f);
   const int32_t mVoxelsInBlock = 32;
   const int32_t mVoxelSize = 10;
   const int32_t mViewDistance = 4;
   bool mStaticPosition = true;
   bool mWireframe = false;

   // Compute shader that runs the marching cubes algorithm and writes the output mesh
   // to a vertex buffer
   struct MarchingCubesJob {
      UNIFORM_BLOCK_BEGIN(MarchingInputParameters)
         UNIFORM_PARAM(glm::mat4, projection)
         UNIFORM_PARAM(glm::mat4, view)
         UNIFORM_PARAM(glm::vec4, offsets[8])
         UNIFORM_PARAM(glm::vec4, color)
         UNIFORM_PARAM(float, voxelSize)
         UNIFORM_PARAM(uint32_t, viewDistance)
         UNIFORM_PARAM(uint32_t, voxelsInBlock)
         UNIFORM_PARAM(float, time)
         UNIFORM_PARAM(uint32_t, flatNormals)
      UNIFORM_BLOCK_END()

      UNIFORM_BLOCK_BEGIN(CounterSSBO)
         UNIFORM_PARAM(uint32_t, numVertices)
      UNIFORM_BLOCK_END()

      SharedPtr<Vk::Effect> effect;
      SharedPtr<Utopian::Vk::Texture> edgeTableTexture;
      SharedPtr<Utopian::Vk::Texture> triangleTableTexture;
      MarchingInputParameters inputUBO;
      CounterSSBO counterSSBO;
   } mMarchingCubesJob;

   // Vertex + pixel shader that renders each generated block mesh from the marching cubes job
   struct TerrainJob {
      UNIFORM_BLOCK_BEGIN(VertexInputParameters)
         UNIFORM_PARAM(glm::mat4, projection)
         UNIFORM_PARAM(glm::mat4, view)
         UNIFORM_PARAM(glm::vec4, clippingPlane)
         UNIFORM_PARAM(glm::vec3, eyePos)
         UNIFORM_PARAM(float, pad)
      UNIFORM_BLOCK_END()

      UNIFORM_BLOCK_BEGIN(FragmentInputParameters)
         UNIFORM_PARAM(glm::vec3, brushPos)
         UNIFORM_PARAM(int, mode) // 0 = phong, 1 = normals, 2 = block cells
      UNIFORM_BLOCK_END()

      SharedPtr<Vk::RenderTarget> renderTarget;
      SharedPtr<Vk::Image> colorImage;
      SharedPtr<Vk::Image> positionImage;
      SharedPtr<Vk::Image> depthImage;
      SharedPtr<Vk::Effect> effect;
      SharedPtr<Vk::Effect> effectWireframe;
      SharedPtr<Vk::Semaphore> completedSemaphore;
      VertexInputParameters inputUBO;
      FragmentInputParameters fragmentInputUBO;
   } mTerrainJob;

   // Compute shader that generates 3D noise and writes it to an 3D image
   struct NoiseJob {
      SharedPtr<Vk::Effect> effect;
      SharedPtr<Vk::Image> sdfImage;
      SharedPtr<Vk::Sampler> sampler;
      const uint32_t textureSize = 256;
   } mNoiseJob;

   // Compute shader that modifies the 3D image based on the cursor which then triggers
   // regeneration of nearby blocks to create new meshes
   struct BrushJob {
      UNIFORM_BLOCK_BEGIN(BrushInputParameters)
         UNIFORM_PARAM(glm::ivec3, startCoord)
         UNIFORM_PARAM(float, brushSize)
         UNIFORM_PARAM(float, brushStrength)
         UNIFORM_PARAM(int, mode) // 0 = add, 1 = subtract
         UNIFORM_PARAM(int, textureRegionSize)
      UNIFORM_BLOCK_END()

      SharedPtr<Vk::Effect> effect;
      BrushInputParameters inputUBO;
      const uint32_t textureRegion = 32;
   } mBrushJob;

   // Compute shader that uses the position image output from the terrain job to
   // get the world space position on the terrain under the cursor
   struct IntersectionJob {
      UNIFORM_BLOCK_BEGIN(IntersectionOutputSSBO)
         UNIFORM_PARAM(glm::vec3, brushPos)
      UNIFORM_BLOCK_END()

      UNIFORM_BLOCK_BEGIN(IntersectionInputUBO)
         UNIFORM_PARAM(glm::ivec2, mousePosition)
      UNIFORM_BLOCK_END()

      SharedPtr<Vk::Effect> effect;
      IntersectionOutputSSBO outputSSBO;
      IntersectionInputUBO inputUBO;
      glm::vec3 brushPos;
   } mIntersectionJob;
};
