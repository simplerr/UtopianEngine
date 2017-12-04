#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "vulkan/handles/Buffer.h"
#include "vulkan/TerrainEffect.h"
#include "vulkan/MarchingCubesEffect.h"

/*
	Steps for implementing the marching algorithm:
	
	1/ Rendering to texture
	2/ 3D textures 
	3/ Output points in GS inside the volume
	4/ Writing to a vertex buffer in the geometry shader
	5/ Generate the mesh for a single block 

	The concept from Nvidias article (http://http.developer.nvidia.com/GPUGems3/gpugems3_ch01.html) is to generate a terrain mesh for each block
	and write it to a vertex buffer that can be reused as long as the block is visible. Blocks are only generated when needed, i.e when they get visible.

	It is the pixel shaders job to write the density values for each cell corner to a 3D texture which then is used in the geometry shader.

	Sources:
	http://paulbourke.net/geometry/polygonise/
	http://http.developer.nvidia.com/GPUGems3/gpugems3_ch01.html
	http://www.icare3d.org/codes-and-projects/codes/opengl_geometry_shader_marching_cubes.html
	https://0fps.net/2012/07/12/smooth-voxel-terrain-part-2/
*/

namespace Vulkan
{
	class Buffer;
	class Renderer;
	class Camera;
	class Pipeline;
	class ComputePipeline;
	class PipelineLayout;
	class DescriptorSet;
	class DescriptorSetLayout;
	class CommandBuffer;
	class VertexDescription;
	class Texture;
	class TerrainEffect;
}

class Block;

struct CubeVertex
{
	CubeVertex(glm::vec3 _pos) : pos(_pos) {

	}
	
	CubeVertex(float x, float y, float z) : pos(glm::vec3(x, y, z)) {

	}

	glm::vec3 pos;
};


struct BlockKey
{
	BlockKey(int32_t _x, int32_t _y, int32_t _z)
		: x(_x), y(_y), z(_z) {

	}

	int32_t x, y, z;
};

bool operator<(BlockKey const& a, BlockKey const& b);

class Terrain
{
public:
	Terrain(Vulkan::Renderer* renderer, Vulkan::Camera* camera);
	~Terrain();

	void Update();

	/**  
	* \brief Adds the blocks within the viewing distance range
	*/
	void UpdateBlockList();

	/**  
	* \brief Generates the vertex buffer for newly added or modified blocks
	*/
	void GenerateBlocks(float time);

	void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	Vulkan::Renderer* mRenderer;
	Vulkan::CommandBuffer* mCommandBuffer;
	Vulkan::Camera* mCamera;

	// Experimentation
	Vulkan::TerrainEffect mTerrainEffect;
	Vulkan::MarchingCubesEffect mMarchingCubesEffect;

	//std::vector<Block*> mBlockList;
	const int32_t mVoxelsInBlock = 32;
	const int32_t mVoxelSize = 400;
	const int32_t mViewDistance = 3;

	std::map<BlockKey, Block*> mBlockList;

	struct PushConstantBlock {
		glm::mat4 world;
		glm::mat4 worldInvTranspose;
	};

	bool mUpdateTimer = true;
	bool mDrawGeneratedBuffer = false;
	bool mUseComputeShader = true;
	int mNumVertices = 0;


	void DumpDebug();
};
