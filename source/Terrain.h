#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <vector>
#include <glm/glm.hpp>
#include "vulkan/handles/Buffer.h"

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
	class PipelineLayout;
	class DescriptorSet;
	class DescriptorSetLayout;
	class CommandBuffer;
	class VertexDescription;
	class Texture;
	class BasicEffect;
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

class GeometryUniformBuffer : public Vulkan::ShaderBuffer
{
public:
	virtual void UpdateMemory(VkDevice device)
	{
		// Map uniform buffer and update it
		uint8_t *mapped;
		mBuffer->MapMemory(0, sizeof(data), 0, (void**)&mapped);
		memcpy(mapped, &data, sizeof(data));
		mBuffer->UnmapMemory();
	}

	virtual int GetSize()
	{
		return sizeof(data);
	}

	// Public data members
	struct {
		glm::mat4 projection;
		glm::mat4 view;
		glm::vec4 offsets[8];
		glm::vec4 color;
		float voxelSize;
		float time;
	} data;
};

class Terrain
{
public:
	Terrain(Vulkan::Renderer* renderer, Vulkan::Camera* camera);
	~Terrain();

	void Update();
	void UpdateBlocks();
	void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/* Builds the vertex buffer containing all the input points to the marching cubes algorithm */
	void BuildPointList();
private:
	Vulkan::Renderer* mRenderer;
	Vulkan::CommandBuffer* mCommandBuffer;
	Vulkan::DescriptorPool* mDescriptorPool;
	Vulkan::DescriptorSetLayout* mDescriptorSetLayout;
	Vulkan::DescriptorSet* mDescriptorSet;
	Vulkan::Pipeline* mGeometryPipeline;
	Vulkan::Pipeline* mBasicPipeline;
	Vulkan::PipelineLayout* mPipelineLayout;
	Vulkan::VertexDescription* mVertexDescription;
	Vulkan::Camera* mCamera;
	Vulkan::Texture* mEdgeTableTexture;
	Vulkan::Texture* mTriangleTableTexture;
	GeometryUniformBuffer  mUniformBuffer;

	// Block specific
	Vulkan::DescriptorSetLayout* mBlockDescriptorSetLayout;

	// Experimentation
	Vulkan::BasicEffect* mBasicEffect;

	std::vector<Block*> mBlockList;
	Block* mTestBlock;
	const uint32_t mWorldSize = 2;
	const uint32_t mBlockSize = 32;
	const uint32_t mVoxelSize = 200;

	struct PushConstantBlock {
		glm::mat4 world;
		glm::mat4 worldInvTranspose;
	};

	bool mUpdateTimer = true;
	bool mDrawGeneratedBuffer = false;
	int mNumVertices = 0;

	std::vector<CubeVertex> mPointList;
	Vulkan::Buffer* mMarchingCubesVB;

	const uint32_t NUM_MAX_STORAGE_BUFFERS = 10;

	void DumpDebug();
};
