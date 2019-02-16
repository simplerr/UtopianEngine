#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_RIGHT_HANDED 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <vector>
#include "vulkan/VulkanInclude.h"
#include "vulkan/Vertex.h"
#include "vulkan/handles/Buffer.h"
#include "utility/math/BoundingBox.h"
#include "utility/Common.h"

#define DEFAULT_NORMAL_MAP_TEXTURE "data/textures/flat_normalmap.png"

namespace Utopian::Vk
{
	class Mesh
	{
	public:
		Mesh(Device* device);
		~Mesh();

		void AddVertex(Vertex vertex);
		void AddVertex(float x, float y, float z);
		void AddTriangle(uint32_t v1, uint32_t v2, uint32_t v3);
		void AddQuad(uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4);
		void BuildBuffers(Device* device);
		void BuildBuffers(const std::vector<Vertex>& vertices, std::vector<uint32_t>);

		void LoadTextures(std::string diffusePath, std::string normalPath = DEFAULT_NORMAL_MAP_TEXTURE);

		void SetTexture(Texture* texture);
		VkDescriptorSet GetTextureDescriptorSet();

		BoundingBox GetBoundingBox();

		uint32_t GetNumIndices();

		Buffer* GetVertxBuffer();
		Buffer* GetIndexBuffer();

		std::vector<Vertex> vertexVector;
		std::vector<unsigned int> indexVector;
	private:
		// Creates a DescriptorSet from the diffuse and normal textures that was added to the mesh.
		void CreateDescriptorSets(SharedPtr<DescriptorSetLayout> descriptorSetLayout, SharedPtr<DescriptorPool> descriptorPool);
		
	private:
		SharedPtr<Buffer> mVertexBuffer;
		SharedPtr<Buffer> mIndexBuffer;

		// Note: Currently each block in the terrain is it's own Mesh which means all of them will have
		// their own DescriptorSet even when the texture will not change on a per instance basis as
		// the case is for Actors. 
		// The solution is probabl to keep the terrain blocks as Meshes but they should not be Renderables.
		// The textures and descriptor set of a Mesh will not be created unless LoadTextures() is called
		SharedPtr<DescriptorSet> mTextureDescriptorSet;

		Device* mDevice;
		Texture* mDiffuseTexture;
		Texture* mNormalTexture;

		BoundingBox mBoundingBox;

		uint32_t mIndicesCount;
		uint32_t mVerticesCount;
	};
}
