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
 * The concept from Nvidias article (http://http.developer.nvidia.com/GPUGems3/gpugems3_ch01.html) is to generate a terrain mesh for each block
 * and write it to a vertex buffer that can be reused as long as the block is visible. Blocks are only generated when needed, i.e when they get visible.
 *
 * Sources:
 * http://paulbourke.net/geometry/polygonise/
 * http://http.developer.nvidia.com/GPUGems3/gpugems3_ch01.html
 * http://www.icare3d.org/codes-and-projects/codes/opengl_geometry_shader_marching_cubes.html
 * https://0fps.net/2012/07/12/smooth-voxel-terrain-part-2/
 */
class MarchingCubes
{
public:
	UNIFORM_BLOCK_BEGIN(MarchingInputParameters)
		UNIFORM_PARAM(glm::mat4, projection)
		UNIFORM_PARAM(glm::mat4, view)
		UNIFORM_PARAM(glm::vec4, offsets[8])
		UNIFORM_PARAM(glm::vec4, color)
		UNIFORM_PARAM(float, voxelSize)
		UNIFORM_PARAM(float, time)
	UNIFORM_BLOCK_END()

	UNIFORM_BLOCK_BEGIN(CounterSSBO)
		UNIFORM_PARAM(uint32_t, numVertices)
	UNIFORM_BLOCK_END()

	UNIFORM_BLOCK_BEGIN(TerrainInputParameters)
		UNIFORM_PARAM(glm::mat4, projection)
		UNIFORM_PARAM(glm::mat4, view)
		UNIFORM_PARAM(glm::vec4, clippingPlane)
		UNIFORM_PARAM(glm::vec3, eyePos)
		UNIFORM_PARAM(float, pad)
	UNIFORM_BLOCK_END()

	MarchingCubes(Utopian::Window* window);
	~MarchingCubes();

	void Run();

	void DestroyCallback();
	void UpdateCallback();
	void DrawCallback();

	/** Adds the blocks within the viewing distance range. */
	void UpdateBlockList();

	/** Generates the vertex buffer for newly added or modified blocks. */
	void GenerateBlocks();
	void RenderBlocks();

	void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	void InitResources();
	void InitMarchingCubesEffect(Vk::Device* device, uint32_t width, uint32_t height);
	void InitTerrainEffect(Vk::Device* device, uint32_t width, uint32_t height);
	glm::ivec3 GetBlockCoordinate(glm::vec3 position);

	Vk::VulkanApp* mVulkanApp;
	Utopian::Window* mWindow;
	SharedPtr<MiniCamera> mCamera;

	// Marching cubes
	SharedPtr<Vk::Effect> mMarchingCubesEffect;
	SharedPtr<Utopian::Vk::Texture> mEdgeTableTex;
	SharedPtr<Utopian::Vk::Texture> mTriangleTableTex;
	MarchingInputParameters mMarchingInputParameters;
	CounterSSBO mCounterSSBO;
	const int32_t mVoxelsInBlock = 32;
	const int32_t mVoxelSize = 400;
	const int32_t mViewDistance = 3;
	std::map<BlockKey, Block*> mBlockList;
	//SharedPtr<Utopian::Vk::Texture> texture3d;

	// Terrain
	SharedPtr<Vk::Effect> mTerrainEffect;
	SharedPtr<Vk::CommandBuffer> mTerrainCommandBuffer;
	TerrainInputParameters mTerrainInputParameters;

	const glm::vec3 mOrigin = glm::vec3(256000.0f);
};
